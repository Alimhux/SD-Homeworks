#ifndef HOZON_PAYMENTS_PAYMENT_HANDLER_H
#define HOZON_PAYMENTS_PAYMENT_HANDLER_H

#include <memory>
#include "http_server.h"
#include "repository/account_repository.h"

namespace hozon {
namespace payments {

class PaymentHandler {
public:
    explicit PaymentHandler(std::shared_ptr<AccountRepository> repo);
    ~PaymentHandler();

    // Register routes
    void registerRoutes(HttpServer& server);

    // HTTP handlers
    HttpResponse createAccount(const HttpRequest& req);
    HttpResponse deposit(const HttpRequest& req);
    HttpResponse getBalance(const HttpRequest& req);
    HttpResponse healthCheck(const HttpRequest& req);

private:
    std::shared_ptr<AccountRepository> repo_;

    // Extract user_id from headers
    std::string getUserId(const HttpRequest& req);
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_PAYMENT_HANDLER_H
