#ifndef HOZON_ORDERS_RESULT_CONSUMER_H
#define HOZON_ORDERS_RESULT_CONSUMER_H

#include <memory>
#include <atomic>
#include "repository/order_repository.h"
#include "rabbitmq.h"

namespace hozon {
namespace orders {

// Result Consumer: получает результаты оплаты из RabbitMQ и обновляет статус заказа
class ResultConsumer {
public:
    ResultConsumer(std::shared_ptr<OrderRepository> repo,
                   std::shared_ptr<RabbitMQConsumer> consumer);
    ~ResultConsumer();

    // Начать потребление
    void start(const std::string& queueName);

    // Остановить потребление
    void stop();

    // Проверка состояния работы
    bool isRunning() const;

private:
    std::shared_ptr<OrderRepository> repo_;
    std::shared_ptr<RabbitMQConsumer> consumer_;
    std::atomic<bool> running_;

    // Обработка сообщения с результатом оплаты
    bool processMessage(const std::string& message);
};

} // namespace orders
} // namespace hozon

#endif // HOZON_ORDERS_RESULT_CONSUMER_H
