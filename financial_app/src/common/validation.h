#pragma once

#include <algorithm>
#include <regex>
#include <string>

#include "exceptions.h"

namespace financial {

// Класс для валидации данных
class Validator {
 public:
  static void validateNotEmpty(const std::string& value,
                               const std::string& fieldName) {
    if (value.empty()) {
      throw ValidationException(fieldName + " cannot be empty");
    }
  }

  static void validatePositive(double value, const std::string& fieldName) {
    if (value <= 0) {
      throw ValidationException(fieldName + " must be positive");
    }
  }

  static void validateNonNegative(double value, const std::string& fieldName) {
    if (value < 0) {
      throw ValidationException(fieldName + " cannot be negative");
    }
  }

  static void validateInRange(double value, double min, double max,
                              const std::string& fieldName) {
    if (value < min || value > max) {
      throw ValidationException(fieldName + " must be between " +
                                std::to_string(min) + " and " +
                                std::to_string(max));
    }
  }

  static void validateId(const std::string& id) {
    validateNotEmpty(id, "ID");
    // проверка что символы в id - цифры или латинские буквы
    std::regex idPattern("^[a-zA-Z0-9-]+$");
    if (!std::regex_match(id, idPattern)) {
      throw ValidationException("Invalid ID format");
    }
  }

  static void validateEmail(const std::string& email) {
    std::regex emailPattern(
        R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!std::regex_match(email, emailPattern)) {
      throw ValidationException("Invalid email format");
    }
  }

  static void validateColor(const std::string& color) {
    validateNotEmpty(color, "Color");
    std::regex colorPattern(R"(^#[0-9A-Fa-f]{3}([0-9A-Fa-f]{3})?$)");
    if (!std::regex_match(color, colorPattern)) {
      throw ValidationException(
          "Invalid color format. Expected #RGB or #RRGGBB (e.g., #FF5733, "
          "#abc).");
    }
  }

  static void validateMaxLength(const std::string& value, size_t maxLength,
                                const std::string& fieldName) {
    if (value.length() > maxLength) {
      throw ValidationException(fieldName + " exceeds maximum length of " +
                                std::to_string(maxLength));
    }
  }
};

}  // namespace financial