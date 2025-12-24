#include "repository/order_repository.h"
#include "models/outbox_message.h"
#include "message_types.h"
#include "uuid.h"
#include <iostream>

namespace hozon {
namespace orders {

std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::NEW: return "NEW";
        case OrderStatus::FINISHED: return "FINISHED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

OrderStatus stringToOrderStatus(const std::string& str) {
    if (str == "NEW") return OrderStatus::NEW;
    if (str == "FINISHED") return OrderStatus::FINISHED;
    if (str == "CANCELLED") return OrderStatus::CANCELLED;
    return OrderStatus::NEW;
}

nlohmann::json Order::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"user_id", userId},
        {"amount", amount},
        {"description", description},
        {"status", orderStatusToString(status)},
        {"created_at", createdAt},
        {"updated_at", updatedAt}
    };
}

Order Order::fromDbRow(const std::map<std::string, std::string>& row) {
    Order order;
    order.id = row.at("id");
    order.userId = row.at("user_id");
    order.amount = std::stod(row.at("amount"));
    order.description = row.count("description") ? row.at("description") : "";
    order.status = stringToOrderStatus(row.at("status"));
    order.createdAt = row.count("created_at") ? row.at("created_at") : "";
    order.updatedAt = row.count("updated_at") ? row.at("updated_at") : "";
    return order;
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

OrderRepository::OrderRepository(std::shared_ptr<Database> db) : db_(db) {}

OrderRepository::~OrderRepository() = default;

std::optional<Order> OrderRepository::findById(const std::string& id) {
    auto result = db_->queryParams(
        "SELECT id, user_id, amount, description, status, created_at, updated_at "
        "FROM orders WHERE id = $1",
        id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    return Order::fromDbRow(result[0]);
}

std::vector<Order> OrderRepository::findByUserId(const std::string& userId) {
    auto result = db_->queryParams(
        "SELECT id, user_id, amount, description, status, created_at, updated_at "
        "FROM orders WHERE user_id = $1 ORDER BY created_at DESC",
        userId
    );

    std::vector<Order> orders;
    for (const auto& row : result) {
        orders.push_back(Order::fromDbRow(row));
    }
    return orders;
}

Order OrderRepository::createWithOutbox(const std::string& userId,
                                         double amount,
                                         const std::string& description) {
    // Transactional Outbox: заказ и сообщение в одной транзакции
    auto& conn = db_->getConnection();
    pqxx::work txn(conn);

    try {
        std::string orderId = UUID::generate();
        std::string outboxId = UUID::generate();
        std::string messageId = UUID::generate();

        txn.exec_params(
            "INSERT INTO orders (id, user_id, amount, description, status) "
            "VALUES ($1, $2, $3, $4, 'NEW')",
            orderId, userId, amount, description
        );

        PaymentRequest paymentReq;
        paymentReq.messageId = messageId;
        paymentReq.orderId = orderId;
        paymentReq.userId = userId;
        paymentReq.amount = amount;

        txn.exec_params(
            "INSERT INTO outbox (id, aggregate_id, event_type, payload, processed) "
            "VALUES ($1, $2, $3, $4, FALSE)",
            outboxId,
            orderId,
            eventTypeToString(EventType::PAYMENT_REQUEST),
            paymentReq.toJson().dump()
        );

        txn.commit();
        std::cout << "Created order " << orderId << " with outbox message" << std::endl;
        return findById(orderId).value();

    } catch (const std::exception& e) {
        txn.abort();
        std::cerr << "Error creating order with outbox: " << e.what() << std::endl;
        throw;
    }
}

bool OrderRepository::updateStatus(const std::string& orderId, OrderStatus status) {
    int affected = db_->executeParams(
        "UPDATE orders SET status = $1, updated_at = NOW() WHERE id = $2",
        orderStatusToString(status), orderId
    );
    return affected > 0;
}

std::vector<std::map<std::string, std::string>> OrderRepository::getUnprocessedOutbox(int limit) {
    auto& conn = db_->getConnection();
    pqxx::work txn(conn);

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

bool OrderRepository::markOutboxProcessed(const std::string& id) {
    int affected = db_->executeParams(
        "UPDATE outbox SET processed = TRUE WHERE id = $1",
        id
    );
    return affected > 0;
}

} // namespace orders
} // namespace hozon
