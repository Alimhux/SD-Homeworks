#ifndef HOZON_CONFIG_H
#define HOZON_CONFIG_H

#include <string>
#include <cstdlib>

namespace hozon {
namespace config {

inline std::string getEnv(const std::string& key, const std::string& defaultValue) {
    const char* value = std::getenv(key.c_str());
    return value ? std::string(value) : defaultValue;
}

inline int getEnvInt(const std::string& key, int defaultValue) {
    const char* value = std::getenv(key.c_str());
    return value ? std::stoi(value) : defaultValue;
}

// Конфигурация базы данных
inline std::string getDbHost() { return getEnv("DB_HOST", "localhost"); }
inline int getDbPort() { return getEnvInt("DB_PORT", 5432); }
inline std::string getDbName() { return getEnv("DB_NAME", "hozon"); }
inline std::string getDbUser() { return getEnv("DB_USER", "postgres"); }
inline std::string getDbPassword() { return getEnv("DB_PASSWORD", "postgres"); }

// Конфигурация RabbitMQ
inline std::string getRabbitHost() { return getEnv("RABBITMQ_HOST", "localhost"); }
inline int getRabbitPort() { return getEnvInt("RABBITMQ_PORT", 5672); }
inline std::string getRabbitUser() { return getEnv("RABBITMQ_USER", "guest"); }
inline std::string getRabbitPassword() { return getEnv("RABBITMQ_PASSWORD", "guest"); }

// URL сервисов (для API Gateway)
inline std::string getOrdersServiceUrl() { return getEnv("ORDERS_SERVICE_URL", "http://localhost:8081"); }
inline std::string getPaymentsServiceUrl() { return getEnv("PAYMENTS_SERVICE_URL", "http://localhost:8082"); }

} // namespace config
} // namespace hozon

#endif // HOZON_CONFIG_H
