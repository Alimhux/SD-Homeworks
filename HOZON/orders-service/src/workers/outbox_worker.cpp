#include "workers/outbox_worker.h"
#include <iostream>
#include <chrono>

namespace hozon {
namespace orders {

OutboxWorker::OutboxWorker(std::shared_ptr<OrderRepository> repo,
                           std::shared_ptr<RabbitMQPublisher> publisher)
    : repo_(repo), publisher_(publisher), running_(false) {}

OutboxWorker::~OutboxWorker() {
    stop();
}

void OutboxWorker::start(const std::string& queueName) {
    if (running_) return;

    queueName_ = queueName;
    running_ = true;
    publisher_->declareQueue(queueName);
    workerThread_ = std::thread(&OutboxWorker::run, this);

    std::cout << "Orders Outbox Worker started, publishing to: " << queueName << std::endl;
}

void OutboxWorker::stop() {
    if (running_) {
        running_ = false;
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
}

bool OutboxWorker::isRunning() const {
    return running_;
}

void OutboxWorker::run() {
    while (running_) {
        try {
            auto messages = repo_->getUnprocessedOutbox(100);

            for (const auto& msg : messages) {
                std::string id = msg.at("id");
                std::string payload = msg.at("payload");
                std::string eventType = msg.at("event_type");
                std::string orderId = msg.at("aggregate_id");

                if (publisher_->publish(queueName_, payload)) {
                    repo_->markOutboxProcessed(id);
                    std::cout << "Published payment request for order: " << orderId << std::endl;
                } else {
                    std::cerr << "Failed to publish payment request for order: " << orderId << std::endl;
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Outbox Worker error: " << e.what() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace orders
} // namespace hozon
