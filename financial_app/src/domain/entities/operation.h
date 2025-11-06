#pragma once

#include "common/types.h"
#include "common/utils.h"
#include "common/validation.h"
#include "domain/value_objects/date_range.h"
#include "domain/value_objects/money.h"
#include "domain/value_objects/types.h"

namespace financial::domain {

// Класс операции - представляет собой транзакцию между счетами
class Operation : public Entity<Operation> {
 private:
  OperationType type_;
  Id bankAccountId_;
  Money amount_;
  DateTime date_;
  std::string description_;
  Id categoryId_;
  DateTime createdAt_;
  DateTime updatedAt_;
  bool isRecurring_;
  std::string recurringPattern_;  // e.g., "MONTHLY", "WEEKLY", "YEARLY"

 public:
  Operation(const Id& id, OperationType type, const Id& bankAccountId,
            const Money& amount, const DateTime& date, const Id& categoryId,
            const std::string& description = "", bool isRecurring = false,
            const std::string& recurringPattern = "")
      : Entity(id),
        type_(type),
        bankAccountId_(bankAccountId),
        amount_(amount),
        date_(date),
        description_(description),
        categoryId_(categoryId),
        createdAt_(DateTimeUtils::now()),
        updatedAt_(DateTimeUtils::now()),
        isRecurring_(isRecurring),
        recurringPattern_(recurringPattern) {
    validate();
  }

  // Геттеры
  OperationType getType() const { return type_; }
  const Id& getBankAccountId() const { return bankAccountId_; }
  const Money& getAmount() const { return amount_; }
  const DateTime& getDate() const { return date_; }
  const std::string& getDescription() const { return description_; }
  const Id& getCategoryId() const { return categoryId_; }
  const DateTime& getCreatedAt() const { return createdAt_; }
  const DateTime& getUpdatedAt() const { return updatedAt_; }
  bool getIsRecurring() const { return isRecurring_; }
  const std::string& getRecurringPattern() const { return recurringPattern_; }

  // Сеттеры с валидацией
  void setAmount(const Money& amount) {
    if (!amount.isPositive()) {
      throw ValidationException("Operation amount must be positive");
    }
    amount_ = amount;
    updateTimestamp();
  }

  void setDate(const DateTime& date) {
    date_ = date;
    updateTimestamp();
  }

  void setDescription(const std::string& description) {
    Validator::validateMaxLength(description, 500, "Operation description");
    description_ = description;
    updateTimestamp();
  }

  void setCategoryId(const Id& categoryId) {
    Validator::validateId(categoryId);
    categoryId_ = categoryId;
    updateTimestamp();
  }

  void setRecurring(bool isRecurring, const std::string& pattern = "") {
    isRecurring_ = isRecurring;
    recurringPattern_ = pattern;
    updateTimestamp();
  }

  // Методы для бизнес-логики приложения
  bool isIncome() const { return type_ == OperationType::INCOME; }

  bool isExpense() const { return type_ == OperationType::EXPENSE; }

  bool isInDateRange(const DateRange& range) const {
    return range.contains(date_);
  }

  Money getSignedAmount() const {
    if (isExpense()) {
      return amount_.multiply(-1);
    }
    return amount_;
  }

  // Операция клонирования для повторяющихся транзакций
  Operation cloneForDate(const DateTime& newDate) const {
    return Operation(IdGenerator::generate("OP"), type_, bankAccountId_,
                     amount_, newDate, categoryId_,
                     description_ + " (Recurring)", false, "");
  }

  static Operation createIncome(const Id& bankAccountId, const Money& amount,
                                const Id& categoryId,
                                const std::string& description = "") {
    return Operation(IdGenerator::generate("OP"), OperationType::INCOME,
                     bankAccountId, amount, DateTimeUtils::now(), categoryId,
                     description);
  }

  static Operation createExpense(const Id& bankAccountId, const Money& amount,
                                 const Id& categoryId,
                                 const std::string& description = "") {
    return Operation(IdGenerator::generate("OP"), OperationType::EXPENSE,
                     bankAccountId, amount, DateTimeUtils::now(), categoryId,
                     description);
  }

 private:
  void validate() {
    Validator::validateId(id_);
    Validator::validateId(bankAccountId_);
    Validator::validateId(categoryId_);
    if (!amount_.isPositive()) {
      throw ValidationException("Operation amount must be positive");
    }
    Validator::validateMaxLength(description_, 500, "Operation description");
  }

  void updateTimestamp() { updatedAt_ = DateTimeUtils::now(); }
};

}  // namespace financial::domain