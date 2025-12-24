#include <iostream>
#include <memory>
#include <csignal>
#include <thread>
#include <chrono>

#include "config.h"
#include "database.h"
#include "rabbitmq.h"
#include "http_server.h"
#include "message_types.h"
#include "handlers/order_handler.h"
#include "repository/order_repository.h"
#include "workers/outbox_worker.h"
#include "workers/result_consumer.h"

using namespace hozon;
using namespace hozon::orders;

// Глобальный флаг для корректного завершения
static std::atomic<bool> g_running{true};

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    g_running = false;
}

void waitForRabbitMQ(const std::string& host, int port, int maxRetries = 30) {
    std::cout << "Waiting for RabbitMQ at " << host << ":" << port << "..." << std::endl;

    for (int i = 0; i < maxRetries; ++i) {
        try {
            RabbitMQPublisher testConn(host, port,
                                       config::getRabbitUser(),
                                       config::getRabbitPassword());
            if (testConn.isConnected()) {
                std::cout << "RabbitMQ is ready!" << std::endl;
                return;
            }
        } catch (...) {}

        std::cout << "RabbitMQ not ready, retrying in 2s... (" << (i + 1) << "/" << maxRetries << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    throw std::runtime_error("Could not connect to RabbitMQ after " + std::to_string(maxRetries) + " retries");
}

void waitForDatabase(const std::string& host, int port, const std::string& dbname,
                     const std::string& user, const std::string& password, int maxRetries = 30) {
    std::cout << "Waiting for PostgreSQL at " << host << ":" << port << "..." << std::endl;

    for (int i = 0; i < maxRetries; ++i) {
        try {
            Database testDb(host, port, dbname, user, password);
            if (testDb.isConnected()) {
                std::cout << "PostgreSQL is ready!" << std::endl;
                return;
            }
        } catch (...) {}

        std::cout << "PostgreSQL not ready, retrying in 2s... (" << (i + 1) << "/" << maxRetries << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    throw std::runtime_error("Could not connect to PostgreSQL after " + std::to_string(maxRetries) + " retries");
}

int main() {
    std::cout << "=== Orders Service ===" << std::endl;

    // Регистрация обработчиков сигналов
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Конфигурация
        int httpPort = config::getEnvInt("HTTP_PORT", 8081);
        std::string dbHost = config::getDbHost();
        int dbPort = config::getDbPort();
        std::string dbName = config::getEnv("DB_NAME", "orders");
        std::string dbUser = config::getEnv("DB_USER", "orders");
        std::string dbPassword = config::getEnv("DB_PASSWORD", "orders");
        std::string rabbitHost = config::getRabbitHost();
        int rabbitPort = config::getRabbitPort();

        // Ожидание зависимостей
        waitForDatabase(dbHost, dbPort, dbName, dbUser, dbPassword);
        waitForRabbitMQ(rabbitHost, rabbitPort);

        // Инициализация подключения к БД
        auto db = std::make_shared<Database>(dbHost, dbPort, dbName, dbUser, dbPassword);

        // Инициализация репозитория
        auto repo = std::make_shared<OrderRepository>(db);

        // Инициализация подключений к RabbitMQ
        auto publisher = std::make_shared<RabbitMQPublisher>(
            rabbitHost, rabbitPort,
            config::getRabbitUser(),
            config::getRabbitPassword()
        );

        auto consumer = std::make_shared<RabbitMQConsumer>(
            rabbitHost, rabbitPort,
            config::getRabbitUser(),
            config::getRabbitPassword()
        );

        // Инициализация воркеров
        OutboxWorker outboxWorker(repo, publisher);
        ResultConsumer resultConsumer(repo, consumer);

        // Запуск воркеров
        outboxWorker.start(queues::PAYMENT_REQUESTS);
        resultConsumer.start(queues::PAYMENT_RESULTS);

        // Инициализация HTTP сервера
        HttpServer server(httpPort);
        OrderHandler handler(repo);
        handler.registerRoutes(server);

        // Запуск HTTP сервера
        server.start();

        std::cout << "Orders Service is running on port " << httpPort << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        // Основной цикл
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Корректное завершение
        std::cout << "Stopping services..." << std::endl;
        server.stop();
        outboxWorker.stop();
        resultConsumer.stop();

        std::cout << "Orders Service stopped." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
