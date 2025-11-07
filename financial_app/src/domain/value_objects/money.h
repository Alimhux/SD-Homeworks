#pragma once

#include <cmath>

#include "common/types.h"
#include "common/validation.h"

namespace financial::domain {

// Объект денежной стоимости
class Money : public ValueObject {
  Decimal amount_;
  std::string currency_;

 public:
  Money() = default;

  explicit Money(Decimal amount, const std::string& currency = "RUB")
      : amount_(amount), currency_(currency) {
    Validator::validateNotEmpty(currency, "Currency");
    Validator::validateMaxLength(currency, 3, "Currency");
  }

  Decimal getAmount() const { return amount_; }
  const std::string& getCurrency() const { return currency_; }

  Money add(const Money& other) const {
    if (currency_ != other.currency_) {
      throw ValidationException("Cannot add money with different currencies");
    }
    return Money(amount_ + other.amount_, currency_);
  }

  Money subtract(const Money& other) const {
    if (currency_ != other.currency_) {
      throw ValidationException(
          "Cannot subtract money with different currencies");
    }
    return Money(amount_ - other.amount_, currency_);
  }

  Money multiply(double factor) const {
    return Money(amount_ * factor, currency_);
  }

  bool isPositive() const { return amount_ > 0; }
  bool isNegative() const { return amount_ < 0; }
  bool isZero() const { return std::abs(amount_) < 0.001; }

  bool equals(const ValueObject& other) const override {
    auto* otherMoney = dynamic_cast<const Money*>(&other);
    if (!otherMoney) return false;
    return std::abs(amount_ - otherMoney->amount_) < 0.001 &&
           currency_ == otherMoney->currency_;
  }

  bool operator==(const Money& other) const { return equals(other); }

  bool operator!=(const Money& other) const { return !equals(other); }

  bool operator<(const Money& other) const {
    if (currency_ != other.currency_) {
      throw ValidationException(
          "Cannot compare money with different currencies");
    }
    return amount_ < other.amount_;
  }

  bool operator>(const Money& other) const { return other < *this; }

  bool operator<=(const Money& other) const { return !(*this > other); }

  bool operator>=(const Money& other) const { return !(*this < other); }

  static Money zero(const std::string& currency = "RUB") {
    return Money(0, currency);
  }
};

}  // namespace financial::domain