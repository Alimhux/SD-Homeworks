#include "repository/account_repository.h"
#include "models/inbox_message.h"
#include "models/outbox_message.h"
#include "uuid.h"
#include <iostream>

namespace hozon {
namespace payments {

nlohmann::json Account::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"user_id", userId},
        {"balance", balance},
        {"version", version},
        {"created_at", createdAt},
        {"updated_at", updatedAt}
    };
}

Account Account::fromDbRow(const std::map<std::string, std::string>& row) {
    Account acc;
    acc.id = row.at("id");
    acc.userId = row.at("user_id");
    acc.balance = std::stod(row.at("balance"));
    acc.version = std::stoi(row.at("version"));
    acc.createdAt = row.count("created_at") ? row.at("created_at") : "";
    acc.updatedAt = row.count("updated_at") ? row.at("updated_at") : "";
    return acc;
}

nlohmann::json Transaction::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"order_id", orderId},
        {"account_id", accountId},
        {"amount", amount},
        {"type", type},
        {"created_at", createdAt}
    };
}

InboxMessage InboxMessage::fromDbRow(const std::map<std::string, std::string>& row) {
    InboxMessage msg;
    msg.id = row.at("id");
    msg.messageId = row.at("message_id");
    msg.payload = row.at("payload");
    msg.processed = row.at("processed") == "t" || row.at("processed") == "true";
    msg.createdAt = row.count("created_at") ? row.at("created_at") : "";
    return msg;
}

OutboxMessage OutboxMessage::fromDbRow(const std::map<std::string, std::string>& row) {
    OutboxMessage msg;
    msg.id = row.at("id");
    msg.aggregateId = row.at("aggregate_id");
    msg.eventType = row.at("event_type");
    msg.payload = row.at("payload");
    msg.processed = row.at("processed") == "t" || row.at("processed") == "true";
    msg.createdAt = row.count("created_at") ? row.at("created_at") : "";
    return msg;
}

AccountRepository::AccountRepository(std::shared_ptr<Database> db) : db_(db) {}

AccountRepository::~AccountRepository() = default;

std::optional<Account> AccountRepository::findByUserId(const std::string& userId) {
    auto result = db_->queryParams(
        "SELECT id, user_id, balance, version, created_at, updated_at "
        "FROM accounts WHERE user_id = $1",
        userId
    );

    if (result.empty()) {
        return std::nullopt;
    }

    return Account::fromDbRow(result[0]);
}

std::optional<Account> AccountRepository::findById(const std::string& id) {
    auto result = db_->queryParams(
        "SELECT id, user_id, balance, version, created_at, updated_at "
        "FROM accounts WHERE id = $1",
        id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    return Account::fromDbRow(result[0]);
}

Account AccountRepository::create(const std::string& userId) {
    std::string id = UUID::generate();

    db_->executeParams(
        "INSERT INTO accounts (id, user_id, balance, version) "
        "VALUES ($1, $2, 0.00, 0)",
        id, userId
    );

    return findById(id).value();
}

bool AccountRepository::deposit(const std::string& userId, double amount) {
    int affected = db_->executeParams(
        "UPDATE accounts SET balance = balance + $1, "
        "version = version + 1, updated_at = NOW() "
        "WHERE user_id = $2",
        amount, userId
    );

    return affected > 0;
}

bool AccountRepository::debitWithCAS(const std::string& userId,
                                      double amount,
                                      const std::string& orderId,
                                      int expectedVersion) {
    auto& conn = db_->getConnection();
    pqxx::work txn(conn);

    try {
        // Проверка идемпотентности: пропустить если уже обработано
        auto existsResult = txn.exec_params(
            "SELECT 1 FROM transactions WHERE order_id = $1",
            orderId
        );

        if (!existsResult.empty()) {
            std::cout << "Transaction for order " << orderId << " already exists (idempotent)" << std::endl;
            txn.commit();
            return true;
        }

        // CAS: обновление только при совпадении версии и достаточном балансе
        auto updateResult = txn.exec_params(
            "UPDATE accounts SET "
            "balance = balance - $1, "
            "version = version + 1, "
            "updated_at = NOW() "
            "WHERE user_id = $2 AND version = $3 AND balance >= $1 "
            "RETURNING id",
            amount, userId, expectedVersion
        );

        if (updateResult.empty()) {
            std::cout << "CAS failed for user " << userId
                     << " (version conflict or insufficient funds)" << std::endl;
            txn.abort();
            return false;
        }

        std::string accountId = updateResult[0][0].as<std::string>();

        std::string txnId = UUID::generate();
        txn.exec_params(
            "INSERT INTO transactions (id, order_id, account_id, amount, type) "
            "VALUES ($1, $2, $3, $4, 'DEBIT')",
            txnId, orderId, accountId, amount
        );

        txn.commit();
        std::cout << "Successfully debited " << amount << " from user " << userId
                 << " for order " << orderId << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error in debitWithCAS: " << e.what() << std::endl;
        txn.abort();
        return false;
    }
}

bool AccountRepository::transactionExists(const std::string& orderId) {
    auto result = db_->queryParams(
        "SELECT 1 FROM transactions WHERE order_id = $1",
        orderId
    );
    return !result.empty();
}

bool AccountRepository::saveToInbox(const std::string& messageId, const std::string& payload) {
    try {
        std::string id = UUID::generate();
        db_->executeParams(
            "INSERT INTO inbox (id, message_id, payload, processed) "
            "VALUES ($1, $2, $3, FALSE) "
            "ON CONFLICT (message_id) DO NOTHING",
            id, messageId, payload
        );
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving to inbox: " << e.what() << std::endl;
        return false;
    }
}

bool AccountRepository::isMessageProcessed(const std::string& messageId) {
    auto result = db_->queryParams(
        "SELECT processed FROM inbox WHERE message_id = $1",
        messageId
    );

    if (result.empty()) {
        return false;
    }

    return result[0]["processed"] == "t" || result[0]["processed"] == "true";
}

bool AccountRepository::markInboxProcessed(const std::string& messageId) {
    int affected = db_->executeParams(
        "UPDATE inbox SET processed = TRUE WHERE message_id = $1",
        messageId
    );
    return affected > 0;
}

void AccountRepository::saveToOutbox(const std::string& aggregateId,
                                      const std::string& eventType,
                                      const std::string& payload) {
    std::string id = UUID::generate();
    db_->executeParams(
        "INSERT INTO outbox (id, aggregate_id, event_type, payload, processed) "
        "VALUES ($1, $2, $3, $4, FALSE)",
        id, aggregateId, eventType, payload
    );
}

std::vector<std::map<std::string, std::string>> AccountRepository::getUnprocessedOutbox(int limit) {
    auto& conn = db_->getConnection();
    pqxx::work txn(conn);

    // FOR UPDATE SKIP LOCKED предотвращает обработку одних сообщений разными воркерами
    auto result = txn.exec_params(
        "SELECT id, aggregate_id, event_type, payload "
        "FROM outbox WHERE processed = FALSE "
        "ORDER BY created_at "
        "LIMIT $1 "
        "FOR UPDATE SKIP LOCKED",
        limit
    );

    std::vector<std::map<std::string, std::string>> messages;
    for (const auto& row : result) {
        std::map<std::string, std::string> msg;
        msg["id"] = row["id"].as<std::string>();
        msg["aggregate_id"] = row["aggregate_id"].as<std::string>();
        msg["event_type"] = row["event_type"].as<std::string>();
        msg["payload"] = row["payload"].as<std::string>();
        messages.push_back(msg);
    }

    txn.commit();
    return messages;
}

bool AccountRepository::markOutboxProcessed(const std::string& id) {
    int affected = db_->executeParams(
        "UPDATE outbox SET processed = TRUE WHERE id = $1",
        id
    );
    return affected > 0;
}

} // namespace payments
} // namespace hozon
