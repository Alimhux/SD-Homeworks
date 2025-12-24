#include "message_types.h"
#include <stdexcept>

namespace hozon {

std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::PAYMENT_REQUEST: return "PAYMENT_REQUEST";
        case EventType::PAYMENT_SUCCESS: return "PAYMENT_SUCCESS";
        case EventType::PAYMENT_FAILED: return "PAYMENT_FAILED";
        default: return "UNKNOWN";
    }
}

EventType stringToEventType(const std::string& str) {
    if (str == "PAYMENT_REQUEST") return EventType::PAYMENT_REQUEST;
    if (str == "PAYMENT_SUCCESS") return EventType::PAYMENT_SUCCESS;
    if (str == "PAYMENT_FAILED") return EventType::PAYMENT_FAILED;
    throw std::invalid_argument("Unknown event type: " + str);
}

// PaymentRequest
nlohmann::json PaymentRequest::toJson() const {
    return nlohmann::json{
        {"message_id", messageId},
        {"order_id", orderId},
        {"user_id", userId},
        {"amount", amount}
    };
}

PaymentRequest PaymentRequest::fromJson(const nlohmann::json& j) {
    PaymentRequest req;
    req.messageId = j.at("message_id").get<std::string>();
    req.orderId = j.at("order_id").get<std::string>();
    req.userId = j.at("user_id").get<std::string>();
    req.amount = j.at("amount").get<double>();
    return req;
}

// PaymentResult
nlohmann::json PaymentResult::toJson() const {
    return nlohmann::json{
        {"message_id", messageId},
        {"order_id", orderId},
        {"user_id", userId},
        {"success", success},
        {"reason", reason}
    };
}

PaymentResult PaymentResult::fromJson(const nlohmann::json& j) {
    PaymentResult res;
    res.messageId = j.at("message_id").get<std::string>();
    res.orderId = j.at("order_id").get<std::string>();
    res.userId = j.at("user_id").get<std::string>();
    res.success = j.at("success").get<bool>();
    res.reason = j.value("reason", "");
    return res;
}

} // namespace hozon
