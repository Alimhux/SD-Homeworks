#pragma once

#include <memory>
#include <string>
#include <vector>

#include "application/commands/commands.h"
#include "application/decorators/command_decorators.h"
#include "domain/entities/bank_account.h"
#include "domain/factories/entity_factory.h"
#include "domain/repositories/repository_interfaces.h"
#include "infrastructure/di/di_container.h"

namespace financial::application {

using namespace financial::domain;
using namespace financial::infrastructure;

// Фасад управления счетами — упрощает операции со счетами
class AccountFacade {
 private:
  std::shared_ptr<IBankAccountRepository> accountRepo_;
  std::shared_ptr<IEntityFactory> factory_;
  std::shared_ptr<CommandHistory> history_;
  int decorationFlags_;

 public:
  AccountFacade(int decorationFlags = DecoratedCommandFactory::PERFORMANCE |
                                      DecoratedCommandFactory::LOGGING)
      : decorationFlags_(decorationFlags) {
    accountRepo_ = ServiceLocator::get<IBankAccountRepository>();
    factory_ = ServiceLocator::get<IEntityFactory>();
    history_ = std::make_shared<CommandHistory>();
  }

  // Методы создания счета
  std::shared_ptr<BankAccount> createAccount(
      const std::string& name, double initialBalance = 0.0,
      const std::string& currency = "RUB",
      const std::string& accountNumber = "") {
    auto command = std::make_shared<CreateAccountCommand>(
        name, Money(initialBalance, currency), accountNumber);

    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);

    return command->getCreatedAccount();
  }

  std::shared_ptr<BankAccount> createSavingsAccount(
      const std::string& name, const std::string& currency = "RUB") {
    return createAccount(name + " (Сберегательный)", 0.0, currency);
  }

  std::shared_ptr<BankAccount> createCheckingAccount(
      const std::string& name, const std::string& currency = "RUB") {
    return createAccount(name + " (Расчетный)", 0.0, currency);
  }

  // Операции со счетами
  void deposit(const Id& accountId, double amount,
               const std::string& currency = "RUB") {
    // Найти или создать категорию "Пополнение счета"
    auto categoryRepo = ServiceLocator::get<ICategoryRepository>();
    auto category = categoryRepo->findByName("Пополнение счета");

    if (!category) {
      auto newCat = factory_->createCategory(
          CategoryType::INCOME, "Пополнение счета", "Прямое пополнение счета");
      categoryRepo->save(newCat);
      category = newCat;
    }

    // Создать операцию дохода
    auto command = std::make_shared<AddOperationCommand>(
        OperationType::INCOME, accountId, Money(amount, currency),
        (*category)->getId(), "Пополнение счета");

    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);
  }

  void withdraw(const Id& accountId, double amount,
                const std::string& currency = "RUB") {
    // Найти или создать категорию "Снятие со счета"
    auto categoryRepo = ServiceLocator::get<ICategoryRepository>();
    auto category = categoryRepo->findByName("Снятие со счета");

    if (!category) {
      auto newCat = factory_->createCategory(
          CategoryType::EXPENSE, "Снятие со счета", "Прямое снятие со счета");
      categoryRepo->save(newCat);
      category = newCat;
    }

    // Создать операцию расхода
    auto command = std::make_shared<AddOperationCommand>(
        OperationType::EXPENSE, accountId, Money(amount, currency),
        (*category)->getId(), "Снятие со счета");

    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);
  }

  void transfer(const Id& fromAccountId, const Id& toAccountId, double amount,
                const std::string& currency = "RUB") {
    auto command = std::make_shared<TransferCommand>(fromAccountId, toAccountId,
                                                     Money(amount, currency));

    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);
  }

  // Запросы счетов
  std::shared_ptr<BankAccount> getAccount(const Id& accountId) {
    auto result = accountRepo_->findById(accountId);
    return result ? *result : nullptr;
  }

  std::shared_ptr<BankAccount> getAccountByNumber(
      const std::string& accountNumber) {
    auto result = accountRepo_->findByAccountNumber(accountNumber);
    return result ? *result : nullptr;
  }

  std::vector<std::shared_ptr<BankAccount>> getAllAccounts() {
    return accountRepo_->findAll();
  }

  std::vector<std::shared_ptr<BankAccount>> getActiveAccounts() {
    return accountRepo_->findActive();
  }

  // Управление счетами
  void activateAccount(const Id& accountId) {
    auto account = getAccount(accountId);
    if (!account) {
      throw EntityNotFoundException("BankAccount", accountId);
    }

    account->activate();
    accountRepo_->update(account);
  }

  void deactivateAccount(const Id& accountId) {
    auto account = getAccount(accountId);
    if (!account) {
      throw EntityNotFoundException("BankAccount", accountId);
    }

    account->deactivate();
    accountRepo_->update(account);
  }

  void updateAccountName(const Id& accountId, const std::string& newName) {
    auto account = getAccount(accountId);
    if (!account) {
      throw EntityNotFoundException("BankAccount", accountId);
    }

    account->setName(newName);
    accountRepo_->update(account);
  }

  void deleteAccount(const Id& accountId) {
    // Проверить, имеет ли счёт нулевой баланс
    auto account = getAccount(accountId);
    if (!account) {
      throw EntityNotFoundException("BankAccount", accountId);
    }

    if (!account->getBalance().isZero()) {
      throw DomainException("Невозможно удалить счёт с ненулевым балансом");
    }

    accountRepo_->remove(accountId);
  }

  // Операции с балансом
  Money getBalance(const Id& accountId) {
    auto account = getAccount(accountId);
    if (!account) {
      throw EntityNotFoundException("BankAccount", accountId);
    }

    return account->getBalance();
  }

  Money getTotalBalance(const std::string& currency = "RUB") {
    auto accounts = getAllAccounts();
    Money total = Money::zero(currency);

    for (const auto& account : accounts) {
      if (account->getCurrency() == currency && account->getIsActive()) {
        total = total.add(account->getBalance());
      }
    }

    return total;
  }

  // Операции с историей команд
  void undo() { history_->undo(); }

  void redo() { history_->redo(); }

  bool canUndo() const { return history_->canUndo(); }

  bool canRedo() const { return history_->canRedo(); }

  std::vector<std::string> getHistory() const {
    return history_->getHistoryNames();
  }

  void clearHistory() { history_->clear(); }
};

}  // namespace financial::application