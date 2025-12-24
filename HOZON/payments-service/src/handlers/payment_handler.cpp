#include "handlers/payment_handler.h"
#include <iostream>

namespace hozon {
namespace payments {

PaymentHandler::PaymentHandler(std::shared_ptr<AccountRepository> repo)
    : repo_(repo) {}

PaymentHandler::~PaymentHandler() = default;

void PaymentHandler::registerRoutes(HttpServer& server) {
    server.post("/accounts", [this](const HttpRequest& req) {
        return createAccount(req);
    });

    server.post("/accounts/deposit", [this](const HttpRequest& req) {
        return deposit(req);
    });

    server.get("/accounts/balance", [this](const HttpRequest& req) {
        return getBalance(req);
    });

    server.get("/health", [this](const HttpRequest& req) {
        return healthCheck(req);
    });
}

std::string PaymentHandler::getUserId(const HttpRequest& req) {
    // Сначала проверяем заголовок
    auto it = req.headers.find("X-User-Id");
    if (it != req.headers.end() && !it->second.empty()) {
        return it->second;
    }

    // Проверяем query-параметры
    auto qit = req.queryParams.find("user_id");
    if (qit != req.queryParams.end() && !qit->second.empty()) {
        return qit->second;
    }

    // Проверяем тело запроса
    try {
        auto json = req.json();
        if (json.contains("user_id")) {
            return json["user_id"].get<std::string>();
        }
    } catch (...) {}

    return "";
}

HttpResponse PaymentHandler::createAccount(const HttpRequest& req) {
    std::string userId = getUserId(req);

    if (userId.empty()) {
        return HttpResponse::badRequest("user_id is required");
    }

    // Проверяем, существует ли уже аккаунт
    auto existing = repo_->findByUserId(userId);
    if (existing) {
        return HttpResponse::conflict("Account already exists for this user");
    }

    try {
        Account account = repo_->create(userId);
        std::cout << "Created account for user: " << userId << std::endl;

        return HttpResponse::created(account.toJson());
    } catch (const std::exception& e) {
        std::cerr << "Error creating account: " << e.what() << std::endl;
        return HttpResponse::internalError("Failed to create account");
    }
}

HttpResponse PaymentHandler::deposit(const HttpRequest& req) {
    std::string userId = getUserId(req);

    if (userId.empty()) {
        return HttpResponse::badRequest("user_id is required");
    }

    // Парсинг суммы из тела запроса
    double amount = 0.0;
    try {
        auto json = req.json();
        if (!json.contains("amount")) {
            return HttpResponse::badRequest("amount is required");
        }
        amount = json["amount"].get<double>();
    } catch (const std::exception& e) {
        return HttpResponse::badRequest("Invalid JSON body");
    }

    if (amount <= 0) {
        return HttpResponse::badRequest("amount must be positive");
    }

    // Проверяем существование аккаунта
    auto account = repo_->findByUserId(userId);
    if (!account) {
        return HttpResponse::notFound("Account not found for this user");
    }

    try {
        bool success = repo_->deposit(userId, amount);
        if (!success) {
            return HttpResponse::internalError("Failed to deposit");
        }

        // Получаем обновлённый аккаунт
        auto updated = repo_->findByUserId(userId);
        std::cout << "Deposited " << amount << " to user: " << userId
                 << ", new balance: " << updated->balance << std::endl;

        return HttpResponse::ok(updated->toJson());
    } catch (const std::exception& e) {
        std::cerr << "Error depositing: " << e.what() << std::endl;
        return HttpResponse::internalError("Failed to deposit");
    }
}

HttpResponse PaymentHandler::getBalance(const HttpRequest& req) {
    std::string userId = getUserId(req);

    if (userId.empty()) {
        return HttpResponse::badRequest("user_id is required");
    }

    auto account = repo_->findByUserId(userId);
    if (!account) {
        return HttpResponse::notFound("Account not found for this user");
    }

    nlohmann::json response = {
        {"user_id", account->userId},
        {"balance", account->balance}
    };

    return HttpResponse::ok(response);
}

HttpResponse PaymentHandler::healthCheck(const HttpRequest& req) {
    (void)req;  // не используется
    nlohmann::json response = {
        {"status", "healthy"},
        {"service", "payments-service"}
    };
    return HttpResponse::ok(response);
}

} // namespace payments
} // namespace hozon
