#pragma once

#include <string>
#include <vector>

#include "domain/factories/entity_factory.h"
#include "domain/repositories/repository_interfaces.h"
#include "domain/services/domain_services.h"
#include "infrastructure/di/di_container.h"

namespace financial::application {

using namespace financial::domain;
using namespace financial::infrastructure;

// Интерфейс команды
class ICommand {
 public:
  virtual ~ICommand() = default;
  virtual void execute() = 0;
  virtual void undo() = 0;
  virtual std::string getName() const = 0;
  virtual bool canUndo() const { return false; }
};

// Базовая команда с общей функциональностью
class BaseCommand : public ICommand {
 protected:
  std::string name_;
  bool executed_ = false;

 public:
  explicit BaseCommand(const std::string& name) : name_(name) {}

  std::string getName() const override { return name_; }

  void execute() override {
    if (executed_) {
      throw std::runtime_error("Команда уже выполнена: " + name_);
    }
    doExecute();
    executed_ = true;
  }

  void undo() override {
    if (!executed_) {
      throw std::runtime_error("Команда не выполнена: " + name_);
    }
    if (!canUndo()) {
      throw std::runtime_error("Команда не может быть отменена: " + name_);
    }
    doUndo();
    executed_ = false;
  }

 protected:
  virtual void doExecute() = 0;
  virtual void doUndo() {}
};

// Команда создания счёта
class CreateAccountCommand : public BaseCommand {
 private:
  std::string accountName_;
  Money initialBalance_;
  std::string accountNumber_;
  std::shared_ptr<BankAccount> createdAccount_;
  std::shared_ptr<IBankAccountRepository> repository_;
  std::shared_ptr<IEntityFactory> factory_;

 public:
  CreateAccountCommand(const std::string& accountName,
                       const Money& initialBalance = Money::zero(),
                       const std::string& accountNumber = "")
      : BaseCommand("-Создать Счет-"),
        accountName_(accountName),
        initialBalance_(initialBalance),
        accountNumber_(accountNumber) {
    repository_ = ServiceLocator::get<IBankAccountRepository>();
    factory_ = ServiceLocator::get<IEntityFactory>();
  }

  bool canUndo() const override { return true; }

 protected:
  void doExecute() override {
    createdAccount_ = factory_->createBankAccount(accountName_, initialBalance_,
                                                  accountNumber_);
    repository_->save(createdAccount_);
  }

  void doUndo() override {
    if (createdAccount_) {
      repository_->remove(createdAccount_->getId());
      createdAccount_ = nullptr;
    }
  }

 public:
  std::shared_ptr<BankAccount> getCreatedAccount() const {
    return createdAccount_;
  }
};

// Команда создания категории
class CreateCategoryCommand : public BaseCommand {
 private:
  CategoryType type_;
  std::string categoryName_;
  std::string description_;
  std::shared_ptr<Category> createdCategory_;
  std::shared_ptr<ICategoryRepository> repository_;
  std::shared_ptr<IEntityFactory> factory_;

 public:
  CreateCategoryCommand(CategoryType type, const std::string& categoryName,
                        const std::string& description = "")
      : BaseCommand("-Создать Категорию-"),
        type_(type),
        categoryName_(categoryName),
        description_(description) {
    repository_ = ServiceLocator::get<ICategoryRepository>();
    factory_ = ServiceLocator::get<IEntityFactory>();
  }

  bool canUndo() const override { return true; }

 protected:
  void doExecute() override {
    createdCategory_ =
        factory_->createCategory(type_, categoryName_, description_);
    repository_->save(createdCategory_);
  }

  void doUndo() override {
    if (createdCategory_) {
      repository_->remove(createdCategory_->getId());
      createdCategory_ = nullptr;
    }
  }

 public:
  std::shared_ptr<Category> getCreatedCategory() const {
    return createdCategory_;
  }
};

// Команда добавления операции
class AddOperationCommand : public BaseCommand {
 private:
  OperationType type_;
  Id bankAccountId_;
  Money amount_;
  Id categoryId_;
  std::string description_;
  std::shared_ptr<Operation> createdOperation_;
  std::shared_ptr<OperationProcessingService> processingService_;
  std::shared_ptr<IEntityFactory> factory_;
  std::shared_ptr<BankAccount> account_;
  Money previousBalance_;

 public:
  AddOperationCommand(OperationType type, const Id& bankAccountId,
                      const Money& amount, const Id& categoryId,
                      const std::string& description = "")
      : BaseCommand("Добавить Операцию"),
        type_(type),
        bankAccountId_(bankAccountId),
        amount_(amount),
        categoryId_(categoryId),
        description_(description) {
    processingService_ = ServiceLocator::get<OperationProcessingService>();
    factory_ = ServiceLocator::get<IEntityFactory>();
  }

  bool canUndo() const override { return true; }

 protected:
  void doExecute() override {
    auto accountRepo = ServiceLocator::get<IBankAccountRepository>();
    auto accountOpt = accountRepo->findById(bankAccountId_);
    if (!accountOpt) {
      throw EntityNotFoundException("BankAccount", bankAccountId_);
    }
    account_ = *accountOpt;
    previousBalance_ = account_->getBalance();

    createdOperation_ = factory_->createOperation(
        type_, bankAccountId_, amount_, categoryId_, description_);
    processingService_->processOperation(createdOperation_);
  }

  void doUndo() override {
    if (createdOperation_ && account_) {
      // Удалить операцию
      auto operationRepo = ServiceLocator::get<IOperationRepository>();
      operationRepo->remove(createdOperation_->getId());

      // Восстановить баланс счёта
      account_->recalculateBalance(previousBalance_);
      auto accountRepo = ServiceLocator::get<IBankAccountRepository>();
      accountRepo->update(account_);

      createdOperation_ = nullptr;
    }
  }

 public:
  std::shared_ptr<Operation> getCreatedOperation() const {
    return createdOperation_;
  }
};

// Команда перевода
class TransferCommand : public BaseCommand {
 private:
  Id fromAccountId_;
  Id toAccountId_;
  Money amount_;
  std::string description_;
  std::shared_ptr<Operation> withdrawOperation_;
  std::shared_ptr<Operation> depositOperation_;

 public:
  TransferCommand(const Id& fromAccountId, const Id& toAccountId,
                  const Money& amount,
                  const std::string& description = "Перевод")
      : BaseCommand("Перевод"),
        fromAccountId_(fromAccountId),
        toAccountId_(toAccountId),
        amount_(amount),
        description_(description) {}

  bool canUndo() const override { return true; }

 protected:
  void doExecute() override {
    auto accountRepo = ServiceLocator::get<IBankAccountRepository>();
    auto factory = ServiceLocator::get<IEntityFactory>();
    auto operationRepo = ServiceLocator::get<IOperationRepository>();

    auto fromAccountOpt = accountRepo->findById(fromAccountId_);
    auto toAccountOpt = accountRepo->findById(toAccountId_);

    if (!fromAccountOpt || !toAccountOpt) {
      throw EntityNotFoundException(
          "BankAccount", !fromAccountOpt ? fromAccountId_ : toAccountId_);
    }

    auto fromAccount = *fromAccountOpt;
    auto toAccount = *toAccountOpt;

    fromAccount->transfer(*toAccount, amount_);

    accountRepo->update(fromAccount);
    accountRepo->update(toAccount);

    auto categoryRepo = ServiceLocator::get<ICategoryRepository>();
    auto transferCategory = categoryRepo->findByName("Перевод");
    if (!transferCategory) {
      transferCategory = factory->createCategory(CategoryType::EXPENSE, "Перевод", "Переводы между счетами");
      categoryRepo->save(*transferCategory);
    }

    // Операция списания
    withdrawOperation_ = factory->createOperation(
        OperationType::EXPENSE,
        fromAccountId_,
        amount_,
        (*transferCategory)->getId(),
        description_
    );
    operationRepo->save(withdrawOperation_);

    // Операция зачисления
    depositOperation_ = factory->createOperation(
        OperationType::INCOME,
        toAccountId_,
        amount_,
        (*transferCategory)->getId(),
        description_
    );
    operationRepo->save(depositOperation_);
  }

  void doUndo() override {
    auto accountRepo = ServiceLocator::get<IBankAccountRepository>();

    auto fromAccountOpt = accountRepo->findById(fromAccountId_);
    auto toAccountOpt = accountRepo->findById(toAccountId_);

    if (fromAccountOpt && toAccountOpt) {
      auto fromAccount = *fromAccountOpt;
      auto toAccount = *toAccountOpt;

      toAccount->transfer(*fromAccount, amount_);

      accountRepo->update(fromAccount);
      accountRepo->update(toAccount);
    }
  }
};

// История команд для функциональности отмены/повтора
class CommandHistory {
 private:
  std::vector<std::shared_ptr<ICommand>> history_;
  size_t currentIndex_ = 0;

 public:
  void execute(std::shared_ptr<ICommand> command) {
    if (currentIndex_ < history_.size()) {
      history_.erase(history_.begin() + currentIndex_, history_.end());
    }

    command->execute();
    history_.push_back(command);
    currentIndex_ = history_.size();
  }

  void undo() {
    if (canUndo()) {
      auto& command = history_[currentIndex_ - 1];
      if (command->canUndo()) {
        command->undo();
        currentIndex_--;
      }
    }
  }

  void redo() {
    if (canRedo()) {
      auto& command = history_[currentIndex_];
      command->execute();
      currentIndex_++;
    }
  }

  bool canUndo() const {
    return currentIndex_ > 0 && history_[currentIndex_ - 1]->canUndo();
  }

  bool canRedo() const { return currentIndex_ < history_.size(); }

  void clear() {
    history_.clear();
    currentIndex_ = 0;
  }

  std::vector<std::string> getHistoryNames() const {
    std::vector<std::string> names;
    for (const auto& cmd : history_) {
      names.push_back(cmd->getName());
    }
    return names;
  }
};

}  // namespace financial::application