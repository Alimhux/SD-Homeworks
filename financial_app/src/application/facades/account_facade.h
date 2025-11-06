#pragma once

#include <memory>
#include <vector>
#include <string>
#include "domain/entities/bank_account.h"
#include "domain/repositories/repository_interfaces.h"
#include "domain/factories/entity_factory.h"
#include "application/commands/commands.h"
#include "application/decorators/command_decorators.h"
#include "infrastructure/di/di_container.h"

namespace financial::application {

using namespace financial::domain;
using namespace financial::infrastructure;

// Account management facade - simplifies account operations
class AccountFacade {
private:
    std::shared_ptr<IBankAccountRepository> accountRepo_;
    std::shared_ptr<IEntityFactory> factory_;
    std::shared_ptr<CommandHistory> history_;
    int decorationFlags_;

public:
    AccountFacade(int decorationFlags = DecoratedCommandFactory::PERFORMANCE | DecoratedCommandFactory::LOGGING)
        : decorationFlags_(decorationFlags) {
        accountRepo_ = ServiceLocator::get<IBankAccountRepository>();
        factory_ = ServiceLocator::get<IEntityFactory>();
        history_ = std::make_shared<CommandHistory>();
    }

    // Create account methods
    std::shared_ptr<BankAccount> createAccount(
        const std::string& name,
        double initialBalance = 0.0,
        const std::string& currency = "RUB",
        const std::string& accountNumber = "") {

        auto command = std::make_shared<CreateAccountCommand>(
            name, Money(initialBalance, currency), accountNumber);

        auto decoratedCommand = DecoratedCommandFactory::decorate(command, decorationFlags_);
        history_->execute(decoratedCommand);

        return command->getCreatedAccount();
    }

    std::shared_ptr<BankAccount> createSavingsAccount(
        const std::string& name,
        const std::string& currency = "RUB") {

        return createAccount(name + " (Savings)", 0.0, currency);
    }

    std::shared_ptr<BankAccount> createCheckingAccount(
        const std::string& name,
        const std::string& currency = "RUB") {

        return createAccount(name + " (Checking)", 0.0, currency);
    }

    // Account operations
    void deposit(const Id& accountId, double amount, const std::string& currency = "RUB") {
        auto account = getAccount(accountId);
        if (!account) {
            throw EntityNotFoundException("BankAccount", accountId);
        }

        account->deposit(Money(amount, currency));
        accountRepo_->update(account);
    }

    void withdraw(const Id& accountId, double amount, const std::string& currency = "RUB") {
        auto account = getAccount(accountId);
        if (!account) {
            throw EntityNotFoundException("BankAccount", accountId);
        }

        account->withdraw(Money(amount, currency));
        accountRepo_->update(account);
    }

    void transfer(const Id& fromAccountId, const Id& toAccountId,
                 double amount, const std::string& currency = "RUB") {

        auto command = std::make_shared<TransferCommand>(
            fromAccountId, toAccountId, Money(amount, currency));

        auto decoratedCommand = DecoratedCommandFactory::decorate(command, decorationFlags_);
        history_->execute(decoratedCommand);
    }

    // Account queries
    std::shared_ptr<BankAccount> getAccount(const Id& accountId) {
        auto result = accountRepo_->findById(accountId);
        return result ? *result : nullptr;
    }

    std::shared_ptr<BankAccount> getAccountByNumber(const std::string& accountNumber) {
        auto result = accountRepo_->findByAccountNumber(accountNumber);
        return result ? *result : nullptr;
    }

    std::vector<std::shared_ptr<BankAccount>> getAllAccounts() {
        return accountRepo_->findAll();
    }

    std::vector<std::shared_ptr<BankAccount>> getActiveAccounts() {
        return accountRepo_->findActive();
    }

    // Account management
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
        // Check if account has zero balance
        auto account = getAccount(accountId);
        if (!account) {
            throw EntityNotFoundException("BankAccount", accountId);
        }

        if (!account->getBalance().isZero()) {
            throw DomainException("Cannot delete account with non-zero balance");
        }

        accountRepo_->remove(accountId);
    }

    // Balance operations
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

    // Command history operations
    void undo() {
        history_->undo();
    }

    void redo() {
        history_->redo();
    }

    bool canUndo() const {
        return history_->canUndo();
    }

    bool canRedo() const {
        return history_->canRedo();
    }

    std::vector<std::string> getHistory() const {
        return history_->getHistoryNames();
    }

    void clearHistory() {
        history_->clear();
    }
};

} // namespace financial::application