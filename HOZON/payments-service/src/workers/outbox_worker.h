#ifndef HOZON_PAYMENTS_OUTBOX_WORKER_H
#define HOZON_PAYMENTS_OUTBOX_WORKER_H

#include <memory>
#include <atomic>
#include <thread>
#include "repository/account_repository.h"
#include "rabbitmq.h"

namespace hozon {
namespace payments {

// Outbox Worker: опрашивает таблицу outbox и публикует сообщения в RabbitMQ
class OutboxWorker {
public:
    OutboxWorker(std::shared_ptr<AccountRepository> repo,
                 std::shared_ptr<RabbitMQPublisher> publisher);
    ~OutboxWorker();

    // Запуск воркера
    void start(const std::string& queueName);

    // Остановка воркера
    void stop();

    // Проверка состояния работы
    bool isRunning() const;

private:
    std::shared_ptr<AccountRepository> repo_;
    std::shared_ptr<RabbitMQPublisher> publisher_;
    std::atomic<bool> running_;
    std::thread workerThread_;
    std::string queueName_;

    // Основной цикл опроса
    void run();
};

} // namespace payments
} // namespace hozon

#endif // HOZON_PAYMENTS_OUTBOX_WORKER_H
