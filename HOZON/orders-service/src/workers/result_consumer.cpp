#include "workers/result_consumer.h"
#include "message_types.h"
#include "models/order.h"
#include <iostream>

namespace hozon {
namespace orders {

ResultConsumer::ResultConsumer(std::shared_ptr<OrderRepository> repo,
                               std::shared_ptr<RabbitMQConsumer> consumer)
    : repo_(repo), consumer_(consumer), running_(false) {}

ResultConsumer::~ResultConsumer() {
    stop();
}

void ResultConsumer::start(const std::string& queueName) {
    if (running_) return;

    running_ = true;
    consumer_->declareQueue(queueName);
    consumer_->consume(queueName, [this](const std::string& message) {
        return processMessage(message);
    });

    std::cout << "Result Consumer started, consuming from: " << queueName << std::endl;
}

void ResultConsumer::stop() {
    if (running_) {
        running_ = false;
        consumer_->stop();
    }
}

bool ResultConsumer::isRunning() const {
    return running_;
}

bool ResultConsumer::processMessage(const std::string& message) {
    try {
        nlohmann::json j = nlohmann::json::parse(message);
        PaymentResult result = PaymentResult::fromJson(j);

        std::cout << "Received payment result: order=" << result.orderId
                 << ", success=" << (result.success ? "true" : "false")
                 << ", reason=" << result.reason << std::endl;

        // Обновление статуса заказа по результату оплаты
        // Идемпотентно - повторное обновление на тот же статус безопасно
        OrderStatus newStatus = result.success ?
            OrderStatus::FINISHED : OrderStatus::CANCELLED;

        bool updated = repo_->updateStatus(result.orderId, newStatus);

        if (updated) {
            std::cout << "Updated order " << result.orderId
                     << " status to: " << orderStatusToString(newStatus) << std::endl;
        } else {
            std::cout << "Order " << result.orderId << " not found or already updated" << std::endl;
        }

        return true;  // ACK - подтверждаем обработку

    } catch (const std::exception& e) {
        std::cerr << "Error processing payment result: " << e.what() << std::endl;
        return false;  // NACK - вернуть в очередь
    }
}

} // namespace orders
} // namespace hozon
