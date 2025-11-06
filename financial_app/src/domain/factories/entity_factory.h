#pragma once

#include <memory>

#include "common/validation.h"
#include "domain/entities/bank_account.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"

namespace financial::domain {

// Интерфейс абстратной фабрики
class IEntityFactory {
 public:
  virtual ~IEntityFactory() = default;

  virtual std::shared_ptr<BankAccount> createBankAccount(
      const std::string& name, const Money& initialBalance = Money::zero(),
      const std::string& accountNumber = "") = 0;

  virtual std::shared_ptr<Category> createCategory(
      CategoryType type, const std::string& name,
      const std::string& description = "") = 0;

  virtual std::shared_ptr<Operation> createOperation(
      OperationType type, const Id& bankAccountId, const Money& amount,
      const Id& categoryId, const std::string& description = "",
      const DateTime& date = DateTimeUtils::now()) = 0;
};

// Конкретная фабрика с валидацией
class EntityFactory : public IEntityFactory {
 private:
  // Здесь валидацию можно в зависимости от бизнес-задачи
  static constexpr size_t MAX_ACCOUNT_NAME_LENGTH = 100;
  static constexpr size_t MAX_CATEGORY_NAME_LENGTH = 50;
  static constexpr size_t MAX_DESCRIPTION_LENGTH = 500;

 public:
  std::shared_ptr<BankAccount> createBankAccount(
      const std::string& name, const Money& initialBalance = Money::zero(),
      const std::string& accountNumber = "") override {
    Validator::validateNotEmpty(name, "Account name");
    Validator::validateMaxLength(name, MAX_ACCOUNT_NAME_LENGTH, "Account name");
    Validator::validateNonNegative(initialBalance.getAmount(),
                                   "Initial balance");

    auto account = std::make_shared<BankAccount>(
        IdGenerator::generate("ACC"), name, initialBalance, accountNumber);

    return account;
  }

  std::shared_ptr<Category> createCategory(
      CategoryType type, const std::string& name,
      const std::string& description = "") override {
    Validator::validateNotEmpty(name, "Category name");
    Validator::validateMaxLength(name, MAX_CATEGORY_NAME_LENGTH,
                                 "Category name");
    Validator::validateMaxLength(description, MAX_DESCRIPTION_LENGTH,
                                 "Category description");

    auto category = std::make_shared<Category>(IdGenerator::generate("CAT"),
                                               type, name, description);

    return category;
  }

  std::shared_ptr<Operation> createOperation(
      OperationType type, const Id& bankAccountId, const Money& amount,
      const Id& categoryId, const std::string& description = "",
      const DateTime& date = DateTimeUtils::now()) override {
    Validator::validateId(bankAccountId);
    Validator::validateId(categoryId);
    Validator::validatePositive(amount.getAmount(), "Operation amount");
    Validator::validateMaxLength(description, MAX_DESCRIPTION_LENGTH,
                                 "Operation description");

    auto operation = std::make_shared<Operation>(IdGenerator::generate("OP"),
                                                 type, bankAccountId, amount,
                                                 date, categoryId, description);

    return operation;
  }

  // Convenience methods for common scenarios
  std::shared_ptr<BankAccount> createSavingsAccount(
      const std::string& name, const std::string& currency = "RUB") {
    return createBankAccount(name + " (Savings)", Money::zero(currency));
  }

  std::shared_ptr<BankAccount> createCheckingAccount(
      const std::string& name, const std::string& currency = "RUB") {
    return createBankAccount(name + " (Checking)", Money::zero(currency));
  }

  std::shared_ptr<Category> createIncomeCategory(
      const std::string& name, const std::string& description = "") {
    return createCategory(CategoryType::INCOME, name, description);
  }

  std::shared_ptr<Category> createExpenseCategory(
      const std::string& name, const std::string& description = "") {
    return createCategory(CategoryType::EXPENSE, name, description);
  }

  std::shared_ptr<Operation> createIncome(const Id& bankAccountId,
                                          const Money& amount,
                                          const Id& categoryId,
                                          const std::string& description = "") {
    return createOperation(OperationType::INCOME, bankAccountId, amount,
                           categoryId, description);
  }

  std::shared_ptr<Operation> createExpense(
      const Id& bankAccountId, const Money& amount, const Id& categoryId,
      const std::string& description = "") {
    return createOperation(OperationType::EXPENSE, bankAccountId, amount,
                           categoryId, description);
  }
};

// Singleton factory instance
class EntityFactoryProvider {
 private:
  static std::shared_ptr<IEntityFactory> instance_;

 public:
  static std::shared_ptr<IEntityFactory> getInstance() {
    if (!instance_) {
      instance_ = std::make_shared<EntityFactory>();
    }
    return instance_;
  }

  static void setInstance(std::shared_ptr<IEntityFactory> factory) {
    instance_ = factory;
  }
};

inline std::shared_ptr<IEntityFactory> EntityFactoryProvider::instance_ =
    nullptr;

}  // namespace financial::domain