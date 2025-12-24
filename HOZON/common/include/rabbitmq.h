#ifndef HOZON_RABBITMQ_H
#define HOZON_RABBITMQ_H

#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>

namespace hozon {

using MessageCallback = std::function<bool(const std::string& message)>;

class RabbitMQPublisher {
public:
    RabbitMQPublisher(const std::string& host,
                      int port,
                      const std::string& user,
                      const std::string& password);
    ~RabbitMQPublisher();

    void declareQueue(const std::string& queueName);
    bool publish(const std::string& queueName, const std::string& message);
    bool isConnected() const;
    void reconnect();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class RabbitMQConsumer {
public:
    RabbitMQConsumer(const std::string& host,
                     int port,
                     const std::string& user,
                     const std::string& password);
    ~RabbitMQConsumer();

    void declareQueue(const std::string& queueName);
    void consume(const std::string& queueName, MessageCallback callback);
    void stop();
    bool isRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hozon

#endif // HOZON_RABBITMQ_H
