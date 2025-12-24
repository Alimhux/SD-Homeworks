#ifndef HOZON_ORDERS_ORDER_REPOSITORY_H
#define HOZON_ORDERS_ORDER_REPOSITORY_H

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include "database.h"
#include "models/order.h"

namespace hozon {
namespace orders {

class OrderRepository {
public:
    explicit OrderRepository(std::shared_ptr<Database> db);
    ~OrderRepository();

    // Операции с заказами
    std::optional<Order> findById(const std::string& id);
    std::vector<Order> findByUserId(const std::string& userId);

    // Создание заказа с Transactional Outbox (атомарная операция)
    Order createWithOutbox(const std::string& userId,
                           double amount,
                           const std::string& description);

    // Обновление статуса заказа
    bool updateStatus(const std::string& orderId, OrderStatus status);

    // Операции с outbox
    std::vector<std::map<std::string, std::string>> getUnprocessedOutbox(int limit = 100);
    bool markOutboxProcessed(const std::string& id);

private:
    std::shared_ptr<Database> db_;
};

} // namespace orders
} // namespace hozon

#endif // HOZON_ORDERS_ORDER_REPOSITORY_H
