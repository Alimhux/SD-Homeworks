#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

#include "config.h"
#include "http_server.h"
#include "http_client.h"
#include "router.h"

using namespace hozon;
using namespace hozon::gateway;

// Глобальный флаг для корректного завершения
static std::atomic<bool> g_running{true};

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    g_running = false;
}

void waitForService(const std::string& name, const std::string& url, int maxRetries = 30) {
    std::cout << "Waiting for " << name << " at " << url << "..." << std::endl;

    HttpClient client;

    for (int i = 0; i < maxRetries; ++i) {
        try {
            auto resp = client.get(url + "/health");
            if (resp.statusCode == 200) {
                std::cout << name << " is ready!" << std::endl;
                return;
            }
        } catch (...) {}

        std::cout << name << " not ready, retrying in 2s... (" << (i + 1) << "/" << maxRetries << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cout << "Warning: " << name << " may not be ready, continuing anyway..." << std::endl;
}

int main() {
    std::cout << "=== API Gateway ===" << std::endl;

    // Регистрация обработчиков сигналов
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Конфигурация
        int httpPort = config::getEnvInt("HTTP_PORT", 8080);
        std::string ordersServiceUrl = config::getOrdersServiceUrl();
        std::string paymentsServiceUrl = config::getPaymentsServiceUrl();

        std::cout << "Orders Service URL: " << ordersServiceUrl << std::endl;
        std::cout << "Payments Service URL: " << paymentsServiceUrl << std::endl;

        // Ожидание бэкенд-сервисов (опционально)
        waitForService("Orders Service", ordersServiceUrl);
        waitForService("Payments Service", paymentsServiceUrl);

        // Инициализация роутера
        Router router(ordersServiceUrl, paymentsServiceUrl);

        // Инициализация HTTP сервера
        HttpServer server(httpPort);
        router.registerRoutes(server);

        // Запуск HTTP сервера
        server.start();

        std::cout << "API Gateway is running on port " << httpPort << std::endl;
        std::cout << "Routes:" << std::endl;
        std::cout << "  POST /api/orders          -> Create order" << std::endl;
        std::cout << "  GET  /api/orders          -> List orders" << std::endl;
        std::cout << "  GET  /api/orders/{id}     -> Get order status" << std::endl;
        std::cout << "  POST /api/accounts        -> Create account" << std::endl;
        std::cout << "  POST /api/accounts/deposit -> Deposit to account" << std::endl;
        std::cout << "  GET  /api/accounts/balance -> Get balance" << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        // Основной цикл
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Корректное завершение
        std::cout << "Stopping API Gateway..." << std::endl;
        server.stop();

        std::cout << "API Gateway stopped." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
