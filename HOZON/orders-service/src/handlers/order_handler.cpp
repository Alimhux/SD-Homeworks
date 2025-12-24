#include "handlers/order_handler.h"
#include <iostream>

namespace hozon {
namespace orders {

OrderHandler::OrderHandler(std::shared_ptr<OrderRepository> repo)
    : repo_(repo) {}

OrderHandler::~OrderHandler() = default;

void OrderHandler::registerRoutes(HttpServer& server) {
    server.post("/orders", [this](const HttpRequest& req) {
        return createOrder(req);
    });

    server.get("/orders", [this](const HttpRequest& req) {
        return getOrders(req);
    });

    server.get("/orders/{id}", [this](const HttpRequest& req) {
        return getOrderById(req);
    });

    server.get("/health", [this](const HttpRequest& req) {
        return healthCheck(req);
    });
}

std::string OrderHandler::getUserId(const HttpRequest& req) {
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

HttpResponse OrderHandler::createOrder(const HttpRequest& req) {
    std::string userId = getUserId(req);

    if (userId.empty()) {
        return HttpResponse::badRequest("user_id is required");
    }

    // Парсинг деталей заказа из тела
    double amount = 0.0;
    std::string description;

    try {
        auto json = req.json();
        if (!json.contains("amount")) {
            return HttpResponse::badRequest("amount is required");
        }
        amount = json["amount"].get<double>();
        description = json.value("description", "");
    } catch (const std::exception& e) {
        return HttpResponse::badRequest("Invalid JSON body");
    }

    if (amount <= 0) {
        return HttpResponse::badRequest("amount must be positive");
    }

    try {
        // Создание заказа с Transactional Outbox
        Order order = repo_->createWithOutbox(userId, amount, description);

        std::cout << "Created order: " << order.id
                 << " for user: " << userId
                 << " amount: " << amount << std::endl;

        return HttpResponse::created(order.toJson());
    } catch (const std::exception& e) {
        std::cerr << "Error creating order: " << e.what() << std::endl;
        return HttpResponse::internalError("Failed to create order");
    }
}

HttpResponse OrderHandler::getOrders(const HttpRequest& req) {
    std::string userId = getUserId(req);

    if (userId.empty()) {
        return HttpResponse::badRequest("user_id is required");
    }

    try {
        auto orders = repo_->findByUserId(userId);

        nlohmann::json response = nlohmann::json::array();
        for (const auto& order : orders) {
            response.push_back(order.toJson());
        }

        return HttpResponse::ok(response);
    } catch (const std::exception& e) {
        std::cerr << "Error getting orders: " << e.what() << std::endl;
        return HttpResponse::internalError("Failed to get orders");
    }
}

HttpResponse OrderHandler::getOrderById(const HttpRequest& req) {
    std::string orderId = req.pathParams.at("id");

    if (orderId.empty()) {
        return HttpResponse::badRequest("order id is required");
    }

    try {
        auto order = repo_->findById(orderId);
        if (!order) {
            return HttpResponse::notFound("Order not found");
        }

        return HttpResponse::ok(order->toJson());
    } catch (const std::exception& e) {
        std::cerr << "Error getting order: " << e.what() << std::endl;
        return HttpResponse::internalError("Failed to get order");
    }
}

HttpResponse OrderHandler::healthCheck(const HttpRequest& req) {
    (void)req;
    nlohmann::json response = {
        {"status", "healthy"},
        {"service", "orders-service"}
    };
    return HttpResponse::ok(response);
}

} // namespace orders
} // namespace hozon
