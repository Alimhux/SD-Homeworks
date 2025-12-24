#ifndef HOZON_PAYMENTS_INBOX_MESSAGE_H
#define HOZON_PAYMENTS_INBOX_MESSAGE_H

#include <string>

namespace hozon {
namespace payments {

struct InboxMessage {
    std::string id;
    std::string messageId;  // Original message ID for deduplication
    std::string payload;
    bool processed;
    std::string createdAt;

    static InboxMessage fromDbRow(const std::map<std::string, std::string>& row);
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_INBOX_MESSAGE_H
