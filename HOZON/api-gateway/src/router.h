#ifndef HOZON_GATEWAY_ROUTER_H
#define HOZON_GATEWAY_ROUTER_H

#include <string>
#include "http_server.h"
#include "http_client.h"

namespace hozon {
namespace gateway {

class Router {
public:
    Router(const std::string& ordersServiceUrl,
           const std::string& paymentsServiceUrl);
    ~Router();

    // Регистрация маршрутов на HTTP сервере
    void registerRoutes(HttpServer& server);

private:
    std::string ordersServiceUrl_;
    std::string paymentsServiceUrl_;
    HttpClient client_;

    // Обработчики проксирования
    HttpResponse proxyToOrders(const HttpRequest& req, const std::string& path);
    HttpResponse proxyToPayments(const HttpRequest& req, const std::string& path);

    // Конвертация HttpClientResponse в HttpResponse
    HttpResponse convertResponse(const HttpClientResponse& clientResp);

    // Формирование заголовков для проксируемого запроса
    std::map<std::string, std::string> buildProxyHeaders(const HttpRequest& req);
};

} // namespace gateway
} // namespace hozon

#endif // HOZON_GATEWAY_ROUTER_H
