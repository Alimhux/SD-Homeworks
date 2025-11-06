#pragma once

#include <algorithm>
#include <vector>

#include "common/exceptions.h"
#include "common/types.h"
#include "common/utils.h"
#include "common/validation.h"
#include "domain/value_objects/money.h"

namespace financial::domain {

// Объект банковского счёта
class BankAccount : public Entity<BankAccount> {
 private:
  std::string name_;
  Money balance_;
  std::string accountNumber_;
  bool isActive_;
  DateTime createdAt_;
  DateTime updatedAt_;
  std::string currency_;

 public:
  BankAccount(const Id& id, const std::string& name,
              const Money& initialBalance = Money::zero(),
              const std::string& accountNumber = "", bool isActive = true)
      : Entity(id),
        name_(name),
        balance_(initialBalance),
        accountNumber_(accountNumber),
        isActive_(isActive),
        createdAt_(DateTimeUtils::now()),
        updatedAt_(DateTimeUtils::now()),
        currency_(initialBalance.getCurrency()) {
    validate();
  }

  // геттеры
  [[nodiscard]] const std::string& getName() const { return name_; }
  [[nodiscard]] const Money& getBalance() const { return balance_; }
  [[nodiscard]] const std::string& getAccountNumber() const {
    return accountNumber_;
  }
  [[nodiscard]] bool getIsActive() const { return isActive_; }
  [[nodiscard]] const DateTime& getCreatedAt() const { return createdAt_; }
  [[nodiscard]] const DateTime& getUpdatedAt() const { return updatedAt_; }
  [[nodiscard]] const std::string& getCurrency() const { return currency_; }

  // сеттеры
  void setName(const std::string& name) {
    Validator::validateNotEmpty(name, "Account name");
    Validator::validateMaxLength(name, 100, "Account name");
    name_ = name;
    updateTimestamp();
  }

  void setAccountNumber(const std::string& accountNumber) {
    Validator::validateMaxLength(accountNumber, 20, "Account number");
    accountNumber_ = accountNumber;
    updateTimestamp();
  }

  void activate() {
    isActive_ = true;
    updateTimestamp();
  }

  void deactivate() {
    isActive_ = false;
    updateTimestamp();
  }

  void deposit(const Money& amount) {
    if (!isActive_) {
      throw DomainException("Cannot deposit to inactive account");
    }
    if (amount.getCurrency() != currency_) {
      throw ValidationException("Currency mismatch");
    }
    if (!amount.isPositive()) {
      throw ValidationException("Deposit amount must be positive");
    }

    balance_ = balance_.add(amount);
    updateTimestamp();
  }

  void withdraw(const Money& amount) {
    if (!isActive_) {
      throw DomainException("Cannot withdraw from inactive account");
    }
    if (amount.getCurrency() != currency_) {
      throw ValidationException("Currency mismatch");
    }
    if (!amount.isPositive()) {
      throw ValidationException("Withdrawal amount must be positive");
    }
    if (balance_ < amount) {
      throw InsufficientFundsException(amount.getAmount(),
                                       balance_.getAmount());
    }

    balance_ = balance_.subtract(amount);
    updateTimestamp();
  }

  void transfer(BankAccount& targetAccount, const Money& amount) {
    if (!isActive_ || !targetAccount.isActive_) {
      throw DomainException("Both accounts must be active for transfer");
    }
    if (this->id_ == targetAccount.id_) {
      throw ValidationException("Cannot transfer to the same account");
    }

    // Транзакция
    this->withdraw(amount);
    try {
      targetAccount.deposit(amount);
    } catch (...) {
      this->deposit(amount);
      throw;
    }
  }

  [[nodiscard]] bool canWithdraw(const Money& amount) const {
    return isActive_ && balance_ >= amount && amount.getCurrency() == currency_;
  }

  // Пересчёт баланса
  void recalculateBalance(const Money& newBalance) {
    if (newBalance.getCurrency() != currency_) {
      throw ValidationException("Currency mismatch during recalculation");
    }

    auto oldBalance = balance_;
    balance_ = newBalance;
    updateTimestamp();
  }

  // static BankAccount createSavingsAccount(const std::string& name, const
  // std::string& currency = "RUB") {
  //     return BankAccount(
  //         IdGenerator::generate("ACC"),
  //         name + " (Savings)",
  //         Money::zero(currency),
  //         "",
  //         true
  //     );
  // }

  // static BankAccount createCheckingAccount(const std::string& name, const
  // std::string& currency = "RUB") {
  //     return BankAccount(
  //         IdGenerator::generate("ACC"),
  //         name + " (Checking)",
  //         Money::zero(currency),
  //         "",
  //         true
  //     );
  // }

 private:
  void validate() const {
    Validator::validateId(id_);
    Validator::validateNotEmpty(name_, "Account name");
    Validator::validateMaxLength(name_, 100, "Account name");
    Validator::validateMaxLength(accountNumber_, 20, "Account number");
  }

  void updateTimestamp() { updatedAt_ = DateTimeUtils::now(); }
};

}  // namespace financial::domain