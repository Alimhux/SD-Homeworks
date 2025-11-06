#pragma once

#include <map>
#include <memory>
#include <vector>

#include "domain/entities/bank_account.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"
#include "domain/repositories/repository_interfaces.h"
#include "domain/value_objects/date_range.h"
#include "domain/factories/entity_factory.h"

namespace financial::domain {

// Различные аналитики (по категориям, временным периодам и т.д.)
struct CategoryAnalytics {
  Id categoryId;
  std::string categoryName;
  Money totalAmount;
  size_t operationCount;
  double percentage;
};

struct PeriodAnalytics {
  DateRange period;
  Money totalIncome;
  Money totalExpense;
  Money netIncome;
  std::vector<CategoryAnalytics> incomeByCategory;
  std::vector<CategoryAnalytics> expenseByCategory;
};

struct AccountBalance {
  Id accountId;
  std::string accountName;
  Money balance;
  Money calculatedBalance;
  bool hasDiscrepancy;
};

// Сервис аналитики
class AnalyticsService {
 private:
  std::shared_ptr<IOperationRepository> operationRepo_;
  std::shared_ptr<ICategoryRepository> categoryRepo_;

 public:
  AnalyticsService(std::shared_ptr<IOperationRepository> operationRepo,
                   std::shared_ptr<ICategoryRepository> categoryRepo)
      : operationRepo_(operationRepo), categoryRepo_(categoryRepo) {}

  // Посчитать аналитику расходов и доходов за определённый период
  PeriodAnalytics calculatePeriodAnalytics(const DateRange& period) {
    PeriodAnalytics result{};
    result.period = period;
    result.totalIncome = Money::zero();
    result.totalExpense = Money::zero();

    // Получаем операции за период
    auto operations =
        operationRepo_->findByDateRange(period.getStart(), period.getEnd());

    // Группировка по категории операций (доходы/расходы)
    std::map<Id, CategoryAnalytics> incomeMap;
    std::map<Id, CategoryAnalytics> expenseMap;

    for (const auto& op : operations) {
      if (!op->isInDateRange(period)) continue;

      auto& targetMap = op->isIncome() ? incomeMap : expenseMap;
      auto& analytics = targetMap[op->getCategoryId()];

      if (analytics.categoryId.empty()) {
        analytics.categoryId = op->getCategoryId();
        auto category = categoryRepo_->findById(op->getCategoryId());
        analytics.categoryName = category ? (*category)->getName() : "Unknown";
        analytics.totalAmount = Money::zero(op->getAmount().getCurrency());
        analytics.operationCount = 0;
      }

      analytics.totalAmount = analytics.totalAmount.add(op->getAmount());
      analytics.operationCount++;

      if (op->isIncome()) {
        result.totalIncome = result.totalIncome.add(op->getAmount());
      } else {
        result.totalExpense = result.totalExpense.add(op->getAmount());
      }
    }

    for (auto& [id, analytics] : incomeMap) {
      if (!result.totalIncome.isZero()) {
        analytics.percentage = (analytics.totalAmount.getAmount() /
                                result.totalIncome.getAmount()) *
                               100;
      }
      result.incomeByCategory.push_back(analytics);
    }

    for (auto& [id, analytics] : expenseMap) {
      if (!result.totalExpense.isZero()) {
        analytics.percentage = (analytics.totalAmount.getAmount() /
                                result.totalExpense.getAmount()) *
                               100;
      }
      result.expenseByCategory.push_back(analytics);
    }

    result.netIncome = result.totalIncome.subtract(result.totalExpense);

    return result;
  }

  // Получить топ 10 категорий по затратам/доходам
  std::vector<CategoryAnalytics> getTopCategories(const DateRange& period,
                                                  OperationType type,
                                                  size_t limit = 10) {
    auto analytics = calculatePeriodAnalytics(period);
    auto& categories = (type == OperationType::INCOME)
                           ? analytics.incomeByCategory
                           : analytics.expenseByCategory;

    // Sort by amount descending
    std::sort(categories.begin(), categories.end(),
              [](const CategoryAnalytics& a, const CategoryAnalytics& b) {
                return a.totalAmount > b.totalAmount;
              });

    if (categories.size() > limit) {
      categories.resize(limit);
    }

    return categories;
  }
};

class BalanceReconciliationService {
 private:
  std::shared_ptr<IBankAccountRepository> accountRepo_;
  std::shared_ptr<IOperationRepository> operationRepo_;

 public:
  BalanceReconciliationService(
      std::shared_ptr<IBankAccountRepository> accountRepo,
      std::shared_ptr<IOperationRepository> operationRepo)
      : accountRepo_(accountRepo), operationRepo_(operationRepo) {}

  // Проверка что текущий баланс на счёте соответствует
  // проведённым на нём операциям
  AccountBalance checkAccountBalance(const Id& accountId) {
    AccountBalance result{};
    result.accountId = accountId;

    auto account = accountRepo_->findById(accountId);
    if (!account) {
      throw EntityNotFoundException("BankAccount", accountId);
    }

    result.accountName = (*account)->getName();
    result.balance = (*account)->getBalance();

    // Calculate balance from operations
    auto operations = operationRepo_->findByAccount(accountId);
    Money calculated = Money::zero((*account)->getCurrency());

    for (const auto& op : operations) {
      if (op->isIncome()) {
        calculated = calculated.add(op->getAmount());
      } else {
        calculated = calculated.subtract(op->getAmount());
      }
    }

    result.calculatedBalance = calculated;
    result.hasDiscrepancy = !(result.balance == result.calculatedBalance);

    return result;
  }

  // Пересчитать и исправить баланс
  void recalculateBalance(const Id& accountId, bool autoFix = false) {
    auto result = checkAccountBalance(accountId);

    if (result.hasDiscrepancy && autoFix) {
      auto account = accountRepo_->findById(accountId);
      if (account) {
        (*account)->recalculateBalance(result.calculatedBalance);
        accountRepo_->update(*account);
      }
    }
  }

  // Возвращает список объектов AccountBalance, где есть сумма по операциям
  // и предполагаемый баланс
  std::vector<AccountBalance> checkAllBalances() {
    std::vector<AccountBalance> results;
    auto accounts = accountRepo_->findAll();

    for (const auto& account : accounts) {
      results.push_back(checkAccountBalance(account->getId()));
    }

    return results;
  }
};


// Доменный сервис для проведения операций
class OperationProcessingService {
 private:
  std::shared_ptr<IBankAccountRepository> accountRepo_;
  std::shared_ptr<IOperationRepository> operationRepo_;
  std::shared_ptr<IEntityFactory> entityFactory_;

 public:
  OperationProcessingService(
      std::shared_ptr<IBankAccountRepository> accountRepo,
      std::shared_ptr<IOperationRepository> operationRepo,
      std::shared_ptr<IEntityFactory> entityFactory)
      : accountRepo_(accountRepo),
        operationRepo_(operationRepo),
        entityFactory_(entityFactory) {}

  // Выполнение конкретной операции
  void processOperation(std::shared_ptr<Operation> operation) {
    auto account = accountRepo_->findById(operation->getBankAccountId());
    if (!account) {
      throw EntityNotFoundException("BankAccount",
                                    operation->getBankAccountId());
    }

    if (operation->isIncome()) {
      (*account)->deposit(operation->getAmount());
    } else {
      (*account)->withdraw(operation->getAmount());
    }

    operationRepo_->save(operation);
    accountRepo_->update(*account);
  }

  // Обработать регулярные операции
  void processRecurringOperations(const DateTime& currentDate) {
    auto operations = operationRepo_->findWhere(
        [](const Operation& op) { return op.getIsRecurring(); });

    for (const auto& op : operations) {
      auto newOp = std::make_shared<Operation>(op->cloneForDate(currentDate));
      processOperation(newOp);
    }
  }
};

}  // namespace financial::domain