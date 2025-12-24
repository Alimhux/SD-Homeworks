#include "workers/outbox_worker.h"
#include "message_types.h"
#include <iostream>
#include <chrono>

namespace hozon {
namespace payments {

OutboxWorker::OutboxWorker(std::shared_ptr<AccountRepository> repo,
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

    std::cout << "Outbox Worker started, publishing to: " << queueName << std::endl;
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
            // Получение необработанных сообщений из outbox
            auto messages = repo_->getUnprocessedOutbox(100);

            for (const auto& msg : messages) {
                std::string id = msg.at("id");
                std::string payload = msg.at("payload");
                std::string eventType = msg.at("event_type");

                // Публикация в RabbitMQ
                if (publisher_->publish(queueName_, payload)) {
                    // Помечаем обработанным только после успешной публикации
                    repo_->markOutboxProcessed(id);
                    std::cout << "Published outbox message: " << eventType
                             << " (id: " << id << ")" << std::endl;
                } else {
                    std::cerr << "Failed to publish outbox message: " << id << std::endl;
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Outbox Worker error: " << e.what() << std::endl;
        }

        // Интервал опроса: 100мс
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace payments
} // namespace hozon
