#include "router.h"
#include <iostream>

namespace hozon {
namespace gateway {

Router::Router(const std::string& ordersServiceUrl,
               const std::string& paymentsServiceUrl)
    : ordersServiceUrl_(ordersServiceUrl),
      paymentsServiceUrl_(paymentsServiceUrl) {}

Router::~Router() = default;

void Router::registerRoutes(HttpServer& server) {
    // Маршруты Orders Service
    server.post("/api/orders", [this](const HttpRequest& req) {
        return proxyToOrders(req, "/orders");
    });

    server.get("/api/orders", [this](const HttpRequest& req) {
        return proxyToOrders(req, "/orders");
    });

    server.get("/api/orders/{id}", [this](const HttpRequest& req) {
        std::string orderId = req.pathParams.at("id");
        return proxyToOrders(req, "/orders/" + orderId);
    });

    // Маршруты Payments Service
    server.post("/api/accounts", [this](const HttpRequest& req) {
        return proxyToPayments(req, "/accounts");
    });

    server.post("/api/accounts/deposit", [this](const HttpRequest& req) {
        return proxyToPayments(req, "/accounts/deposit");
    });

    server.get("/api/accounts/balance", [this](const HttpRequest& req) {
        return proxyToPayments(req, "/accounts/balance");
    });

    // Эндпоинты проверки здоровья
    server.get("/health", [](const HttpRequest& req) {
        (void)req;
        nlohmann::json response = {
            {"status", "healthy"},
            {"service", "api-gateway"}
        };
        return HttpResponse::ok(response);
    });

    server.get("/api/orders/health", [this](const HttpRequest& req) {
        return proxyToOrders(req, "/health");
    });

    server.get("/api/payments/health", [this](const HttpRequest& req) {
        return proxyToPayments(req, "/health");
    });
}

std::map<std::string, std::string> Router::buildProxyHeaders(const HttpRequest& req) {
    std::map<std::string, std::string> headers;

    // Проксирование заголовка X-User-Id
    auto it = req.headers.find("X-User-Id");
    if (it != req.headers.end()) {
        headers["X-User-Id"] = it->second;
    }

    // Проксирование Content-Type
    auto ct = req.headers.find("Content-Type");
    if (ct != req.headers.end()) {
        headers["Content-Type"] = ct->second;
    } else {
        headers["Content-Type"] = "application/json";
    }

    return headers;
}

HttpResponse Router::proxyToOrders(const HttpRequest& req, const std::string& path) {
    std::string url = ordersServiceUrl_ + path;

    // Добавление query-параметров
    if (!req.queryParams.empty()) {
        url += "?";
        bool first = true;
        for (const auto& [key, value] : req.queryParams) {
            if (!first) url += "&";
            url += key + "=" + value;
            first = false;
        }
    }

    auto headers = buildProxyHeaders(req);

    try {
        HttpClientResponse resp;

        switch (req.method) {
            case HttpMethod::GET:
                resp = client_.get(url, headers);
                break;
            case HttpMethod::POST:
                resp = client_.post(url, req.body, headers);
                break;
            case HttpMethod::PUT:
                resp = client_.put(url, req.body, headers);
                break;
            case HttpMethod::DELETE:
                resp = client_.del(url, headers);
                break;
            default:
                return HttpResponse::badRequest("Method not supported");
        }

        return convertResponse(resp);

    } catch (const std::exception& e) {
        std::cerr << "Error proxying to Orders Service: " << e.what() << std::endl;
        return HttpResponse::internalError("Orders Service unavailable");
    }
}

HttpResponse Router::proxyToPayments(const HttpRequest& req, const std::string& path) {
    std::string url = paymentsServiceUrl_ + path;

    // Добавление query-параметров
    if (!req.queryParams.empty()) {
        url += "?";
        bool first = true;
        for (const auto& [key, value] : req.queryParams) {
            if (!first) url += "&";
            url += key + "=" + value;
            first = false;
        }
    }

    auto headers = buildProxyHeaders(req);

    try {
        HttpClientResponse resp;

        switch (req.method) {
            case HttpMethod::GET:
                resp = client_.get(url, headers);
                break;
            case HttpMethod::POST:
                resp = client_.post(url, req.body, headers);
                break;
            case HttpMethod::PUT:
                resp = client_.put(url, req.body, headers);
                break;
            case HttpMethod::DELETE:
                resp = client_.del(url, headers);
                break;
            default:
                return HttpResponse::badRequest("Method not supported");
        }

        return convertResponse(resp);

    } catch (const std::exception& e) {
        std::cerr << "Error proxying to Payments Service: " << e.what() << std::endl;
        return HttpResponse::internalError("Payments Service unavailable");
    }
}

HttpResponse Router::convertResponse(const HttpClientResponse& clientResp) {
    HttpResponse resp;
    resp.statusCode = clientResp.statusCode;
    resp.body = clientResp.body;
    resp.headers["Content-Type"] = "application/json";
    return resp;
}

} // namespace gateway
} // namespace hozon
