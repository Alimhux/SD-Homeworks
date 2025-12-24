#ifndef HOZON_PAYMENTS_OUTBOX_MESSAGE_H
#define HOZON_PAYMENTS_OUTBOX_MESSAGE_H

#include <string>

namespace hozon {
namespace payments {

struct OutboxMessage {
    std::string id;
    std::string aggregateId;  // order_id
    std::string eventType;    // PAYMENT_SUCCESS or PAYMENT_FAILED
    std::string payload;
    bool processed;
    std::string createdAt;

    static OutboxMessage fromDbRow(const std::map<std::string, std::string>& row);
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_OUTBOX_MESSAGE_H
