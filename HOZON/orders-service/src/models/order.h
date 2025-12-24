#ifndef HOZON_ORDERS_ORDER_H
#define HOZON_ORDERS_ORDER_H

#include <string>
#include <map>
#include "json.h"

namespace hozon {
namespace orders {

// Статус заказа
enum class OrderStatus {
    NEW,        // Создан, ожидает оплаты
    FINISHED,   // Оплата успешна
    CANCELLED   // Оплата не удалась
};

std::string orderStatusToString(OrderStatus status);
OrderStatus stringToOrderStatus(const std::string& str);

struct Order {
    std::string id;
    std::string userId;
    double amount;
    std::string description;
    OrderStatus status;
    std::string createdAt;
    std::string updatedAt;

    nlohmann::json toJson() const;
    static Order fromDbRow(const std::map<std::string, std::string>& row);
};

} // namespace orders
} // namespace hozon

#endif // HOZON_ORDERS_ORDER_H
