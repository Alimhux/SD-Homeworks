#ifndef HOZON_ORDERS_OUTBOX_MESSAGE_H
#define HOZON_ORDERS_OUTBOX_MESSAGE_H

#include <string>
#include <map>

namespace hozon {
namespace orders {

struct OutboxMessage {
    std::string id;
    std::string aggregateId;  // order_id
    std::string eventType;    // PAYMENT_REQUEST
    std::string payload;
    bool processed;
    std::string createdAt;

    static OutboxMessage fromDbRow(const std::map<std::string, std::string>& row);
};

} // namespace orders
} // namespace hozon

#endif // HOZON_ORDERS_OUTBOX_MESSAGE_H
