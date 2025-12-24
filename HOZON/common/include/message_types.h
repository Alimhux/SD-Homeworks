#ifndef HOZON_MESSAGE_TYPES_H
#define HOZON_MESSAGE_TYPES_H

#include <string>
#include "json.h"

namespace hozon {

// Типы событий для очереди сообщений
enum class EventType {
    PAYMENT_REQUEST,
    PAYMENT_SUCCESS,
    PAYMENT_FAILED
};

// Конвертация EventType в строку
std::string eventTypeToString(EventType type);

// Конвертация строки в EventType
EventType stringToEventType(const std::string& str);

// Сообщение запроса оплаты (Orders -> Payments)
struct PaymentRequest {
    std::string messageId;
    std::string orderId;
    std::string userId;
    double amount;

    nlohmann::json toJson() const;
    static PaymentRequest fromJson(const nlohmann::json& j);
};

// Сообщение результата оплаты (Payments -> Orders)
struct PaymentResult {
    std::string messageId;
    std::string orderId;
    std::string userId;
    bool success;
    std::string reason;

    nlohmann::json toJson() const;
    static PaymentResult fromJson(const nlohmann::json& j);
};

// Названия очередей
namespace queues {
    const std::string PAYMENT_REQUESTS = "payment.requests";
    const std::string PAYMENT_RESULTS = "payment.results";
}

} // namespace hozon

#endif // HOZON_MESSAGE_TYPES_H
