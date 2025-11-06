#pragma once

#include <string>

namespace financial::domain {

enum class OperationType {
  INCOME,
  EXPENSE
};

inline std::string operationTypeToString(OperationType type) {
  switch (type) {
    case OperationType::INCOME: return "INCOME";
    case OperationType::EXPENSE: return "EXPENSE";
    default: return "UNKNOWN";
  }
}

inline OperationType stringToOperationType(const std::string& str) {
  if (str == "INCOME") return OperationType::INCOME;
  if (str == "EXPENSE") return OperationType::EXPENSE;
  throw std::invalid_argument("Invalid operation type: " + str);
}

enum class CategoryType {
  INCOME,
  EXPENSE
};

inline std::string categoryTypeToString(CategoryType type) {
  switch (type) {
    case CategoryType::INCOME: return "INCOME";
    case CategoryType::EXPENSE: return "EXPENSE";
    default: return "UNKNOWN";
  }
}

inline CategoryType stringToCategoryType(const std::string& str) {
  if (str == "INCOME") return CategoryType::INCOME;
  if (str == "EXPENSE") return CategoryType::EXPENSE;
  throw std::invalid_argument("Invalid category type: " + str);
}

} // namespace financial::domain