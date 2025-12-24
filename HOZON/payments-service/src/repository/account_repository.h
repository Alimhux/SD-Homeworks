#ifndef HOZON_PAYMENTS_ACCOUNT_REPOSITORY_H
#define HOZON_PAYMENTS_ACCOUNT_REPOSITORY_H

#include <string>
#include <optional>
#include <memory>
#include "database.h"
#include "models/account.h"

namespace hozon {
namespace payments {

class AccountRepository {
public:
    explicit AccountRepository(std::shared_ptr<Database> db);
    ~AccountRepository();

    // Операции с аккаунтами
    std::optional<Account> findByUserId(const std::string& userId);
    std::optional<Account> findById(const std::string& id);
    Account create(const std::string& userId);
    bool deposit(const std::string& userId, double amount);

    // Списание с CAS (Compare-and-Swap) для exactly-once семантики
    // Возвращает true при успехе, false при недостатке средств или конфликте версий
    bool debitWithCAS(const std::string& userId,
                      double amount,
                      const std::string& orderId,
                      int expectedVersion);

    // Проверка существования транзакции (для идемпотентности)
    bool transactionExists(const std::string& orderId);

    // Операции с inbox (Transactional Inbox паттерн)
    bool saveToInbox(const std::string& messageId, const std::string& payload);
    bool isMessageProcessed(const std::string& messageId);
    bool markInboxProcessed(const std::string& messageId);

    // Операции с outbox (Transactional Outbox паттерн)
    void saveToOutbox(const std::string& aggregateId,
                      const std::string& eventType,
                      const std::string& payload);
    std::vector<std::map<std::string, std::string>> getUnprocessedOutbox(int limit = 100);
    bool markOutboxProcessed(const std::string& id);

private:
    std::shared_ptr<Database> db_;
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_ACCOUNT_REPOSITORY_H
