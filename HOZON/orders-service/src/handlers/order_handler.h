#ifndef HOZON_ORDERS_ORDER_HANDLER_H
#define HOZON_ORDERS_ORDER_HANDLER_H

#include <memory>
#include "http_server.h"
#include "repository/order_repository.h"

namespace hozon {
namespace orders {

class OrderHandler {
public:
    explicit OrderHandler(std::shared_ptr<OrderRepository> repo);
    ~OrderHandler();

    // Register routes
    void registerRoutes(HttpServer& server);

    // HTTP handlers
    HttpResponse createOrder(const HttpRequest& req);
    HttpResponse getOrders(const HttpRequest& req);
    HttpResponse getOrderById(const HttpRequest& req);
    HttpResponse healthCheck(const HttpRequest& req);

private:
    std::shared_ptr<OrderRepository> repo_;

    // Extract user_id from request
    std::string getUserId(const HttpRequest& req);
};

} // namespace orders
} // namespace hozon

#endif // HOZON_ORDERS_ORDER_HANDLER_H
