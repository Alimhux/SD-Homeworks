#ifndef HOZON_PAYMENTS_INBOX_WORKER_H
#define HOZON_PAYMENTS_INBOX_WORKER_H

#include <memory>
#include <atomic>
#include <thread>
#include "repository/account_repository.h"
#include "rabbitmq.h"

namespace hozon {
namespace payments {

// Inbox Worker: получает сообщения из RabbitMQ, сохраняет в inbox,
// обрабатывает платежи и записывает результаты в outbox
class InboxWorker {
public:
    InboxWorker(std::shared_ptr<AccountRepository> repo,
                std::shared_ptr<RabbitMQConsumer> consumer);
    ~InboxWorker();

    // Начать получение сообщений
    void start(const std::string& queueName);

    // Остановить воркер
    void stop();

    // Проверка состояния работы
    bool isRunning() const;

private:
    std::shared_ptr<AccountRepository> repo_;
    std::shared_ptr<RabbitMQConsumer> consumer_;
    std::atomic<bool> running_;

    // Обработка запроса на оплату
    bool processMessage(const std::string& message);

    // Обработка платежа и запись результата в outbox
    void processPayment(const std::string& messageId,
                        const std::string& orderId,
                        const std::string& userId,
                        double amount);
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_INBOX_WORKER_H
