#ifndef HOZON_PAYMENTS_ACCOUNT_H
#define HOZON_PAYMENTS_ACCOUNT_H

#include <string>
#include "json.h"

namespace hozon {
namespace payments {

struct Account {
    std::string id;
    std::string userId;
    double balance;
    int version;  // Для оптимистичной блокировки
    std::string createdAt;
    std::string updatedAt;

    nlohmann::json toJson() const;
    static Account fromDbRow(const std::map<std::string, std::string>& row);
};

struct Transaction {
    std::string id;
    std::string orderId;
    std::string accountId;
    double amount;
    std::string type;  // DEBIT или CREDIT
    std::string createdAt;

    nlohmann::json toJson() const;
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_ACCOUNT_H
