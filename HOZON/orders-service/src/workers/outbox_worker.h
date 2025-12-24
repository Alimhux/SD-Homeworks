#ifndef HOZON_ORDERS_OUTBOX_WORKER_H
#define HOZON_ORDERS_OUTBOX_WORKER_H

#include <memory>
#include <atomic>
#include <thread>
#include "repository/order_repository.h"
#include "rabbitmq.h"

namespace hozon {
namespace orders {

// Outbox Worker: опрашивает таблицу outbox и публикует запросы оплаты в RabbitMQ
class OutboxWorker {
public:
    OutboxWorker(std::shared_ptr<OrderRepository> repo,
                 std::shared_ptr<RabbitMQPublisher> publisher);
    ~OutboxWorker();

    // Запуск воркера
    void start(const std::string& queueName);

    // Остановка воркера
    void stop();

    // Проверка состояния работы
    bool isRunning() const;

private:
    std::shared_ptr<OrderRepository> repo_;
    std::shared_ptr<RabbitMQPublisher> publisher_;
    std::atomic<bool> running_;
    std::thread workerThread_;
    std::string queueName_;

    void run();
};

} // namespace orders
} // namespace hozon

#endif // HOZON_ORDERS_OUTBOX_WORKER_H
