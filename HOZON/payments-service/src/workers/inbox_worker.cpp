#include "workers/inbox_worker.h"
#include "message_types.h"
#include "uuid.h"
#include <iostream>

namespace hozon {
namespace payments {

InboxWorker::InboxWorker(std::shared_ptr<AccountRepository> repo,
                         std::shared_ptr<RabbitMQConsumer> consumer)
    : repo_(repo), consumer_(consumer), running_(false) {}

InboxWorker::~InboxWorker() {
    stop();
}

void InboxWorker::start(const std::string& queueName) {
    if (running_) return;

    running_ = true;
    consumer_->declareQueue(queueName);
    consumer_->consume(queueName, [this](const std::string& message) {
        return processMessage(message);
    });

    std::cout << "Inbox Worker started, consuming from: " << queueName << std::endl;
}

void InboxWorker::stop() {
    if (running_) {
        running_ = false;
        consumer_->stop();
    }
}

bool InboxWorker::isRunning() const {
    return running_;
}

bool InboxWorker::processMessage(const std::string& message) {
    try {
        // Парсинг запроса на оплату
        nlohmann::json j = nlohmann::json::parse(message);
        PaymentRequest request = PaymentRequest::fromJson(j);

        std::cout << "Received payment request: order=" << request.orderId
                 << ", user=" << request.userId
                 << ", amount=" << request.amount << std::endl;

        // Шаг 1: Сохранение в inbox (Transactional Inbox)
        // Гарантирует сохранность сообщения
        if (!repo_->saveToInbox(request.messageId, message)) {
            std::cerr << "Failed to save message to inbox" << std::endl;
            return false;  // вернуть в очередь
        }

        // Шаг 2: Проверка обработки (идемпотентность)
        if (repo_->isMessageProcessed(request.messageId)) {
            std::cout << "Message " << request.messageId << " already processed (idempotent)" << std::endl;
            return true;  // ACK - уже обработано
        }

        // Шаг 3: Обработка платежа
        processPayment(request.messageId, request.orderId, request.userId, request.amount);

        // Шаг 4: Пометить inbox как обработанный
        repo_->markInboxProcessed(request.messageId);

        return true;  // ACK - подтверждаем обработку

    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
        return false;  // NACK - вернуть в очередь
    }
}

void InboxWorker::processPayment(const std::string& messageId,
                                  const std::string& orderId,
                                  const std::string& userId,
                                  double amount) {
    PaymentResult result;
    result.messageId = UUID::generate();
    result.orderId = orderId;
    result.userId = userId;

    // Проверяем существование аккаунта
    auto account = repo_->findByUserId(userId);
    if (!account) {
        result.success = false;
        result.reason = "Account not found";
        std::cout << "Payment failed: Account not found for user " << userId << std::endl;
    } else {
        // Списание с CAS
        bool debited = repo_->debitWithCAS(userId, amount, orderId, account->version);

        if (debited) {
            result.success = true;
            result.reason = "Payment successful";
            std::cout << "Payment successful: order=" << orderId
                     << ", amount=" << amount << std::endl;
        } else {
            // Возможен конфликт версий или недостаточно средств
            // Перезапрашиваем аккаунт для проверки
            auto updatedAccount = repo_->findByUserId(userId);
            if (updatedAccount && updatedAccount->balance < amount) {
                result.success = false;
                result.reason = "Insufficient funds";
                std::cout << "Payment failed: Insufficient funds for order " << orderId << std::endl;
            } else if (repo_->transactionExists(orderId)) {
                // Уже обработано (идемпотентно)
                result.success = true;
                result.reason = "Payment already processed";
                std::cout << "Payment already processed for order " << orderId << std::endl;
            } else {
                result.success = false;
                result.reason = "Version conflict, please retry";
                std::cout << "Payment failed: Version conflict for order " << orderId << std::endl;
            }
        }
    }

    // Сохранение результата в outbox (Transactional Outbox)
    std::string eventType = result.success ?
        eventTypeToString(EventType::PAYMENT_SUCCESS) :
        eventTypeToString(EventType::PAYMENT_FAILED);

    repo_->saveToOutbox(orderId, eventType, result.toJson().dump());
    std::cout << "Saved payment result to outbox: " << eventType << std::endl;
}

} // namespace payments
} // namespace hozon
