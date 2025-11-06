#pragma once

#include <vector>

#include "application/commands/commands.h"
#include "application/decorators/command_decorators.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"
#include "domain/repositories/repository_interfaces.h"
#include "domain/services/domain_services.h"

namespace financial::application {

using namespace financial::domain;

// Фасад управления категориями
class CategoryFacade {
 private:
  std::shared_ptr<ICategoryRepository> categoryRepo_;
  std::shared_ptr<IEntityFactory> factory_;
  std::shared_ptr<CommandHistory> history_;
  int decorationFlags_;

 public:
  CategoryFacade(int decorationFlags = DecoratedCommandFactory::LOGGING)
      : decorationFlags_(decorationFlags) {
    categoryRepo_ = ServiceLocator::get<ICategoryRepository>();
    factory_ = ServiceLocator::get<IEntityFactory>();
    history_ = std::make_shared<CommandHistory>();
  }

  // Создать категорию
  std::shared_ptr<Category> createCategory(
      CategoryType type, const std::string& name,
      const std::string& description = "") {
    auto command =
        std::make_shared<CreateCategoryCommand>(type, name, description);
    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);

    return command->getCreatedCategory();
  }

  std::shared_ptr<Category> createIncomeCategory(
      const std::string& name, const std::string& description = "") {
    return createCategory(CategoryType::INCOME, name, description);
  }

  std::shared_ptr<Category> createExpenseCategory(
      const std::string& name, const std::string& description = "") {
    return createCategory(CategoryType::EXPENSE, name, description);
  }

  // Запросы категорий
  std::shared_ptr<Category> getCategory(const Id& categoryId) {
    auto result = categoryRepo_->findById(categoryId);
    return result ? *result : nullptr;
  }

  std::shared_ptr<Category> getCategoryByName(const std::string& name) {
    auto result = categoryRepo_->findByName(name);
    return result ? *result : nullptr;
  }

  std::vector<std::shared_ptr<Category>> getAllCategories() {
    return categoryRepo_->findAll();
  }

  std::vector<std::shared_ptr<Category>> getIncomeCategories() {
    return categoryRepo_->findByType(CategoryType::INCOME);
  }

  std::vector<std::shared_ptr<Category>> getExpenseCategories() {
    return categoryRepo_->findByType(CategoryType::EXPENSE);
  }

  // Управление категориями
  void updateCategory(const Id& categoryId, const std::string& newName,
                      const std::string& newDescription) {
    auto category = getCategory(categoryId);
    if (!category) {
      throw EntityNotFoundException("Category", categoryId);
    }

    category->setName(newName);
    category->setDescription(newDescription);
    categoryRepo_->update(category);
  }

  void deleteCategory(const Id& categoryId) {
    auto operationRepo = ServiceLocator::get<IOperationRepository>();
    auto operations = operationRepo->findByCategory(categoryId);

    if (!operations.empty()) {
      throw DomainException(
          "Невозможно удалить категорию с существующими операциями");
    }

    categoryRepo_->remove(categoryId);
  }

  // Создать стандартные категории
  void createDefaultCategories() {
    // Категории доходов
    createIncomeCategory("Зарплата", "Ежемесячная зарплата");
    createIncomeCategory("Фриланс", "Доход от фриланса");
    createIncomeCategory("Инвестиции", "Доход от инвестиций");
    createIncomeCategory("Подарок", "Подарки и пожертвования");
    createIncomeCategory("Другой доход", "Другие источники дохода");

    // Категории расходов
    createExpenseCategory("Еда", "Продукты и питание");
    createExpenseCategory("Транспорт", "Расходы на транспорт");
    createExpenseCategory("Жилье", "Аренда и коммунальные услуги");
    createExpenseCategory("Здравоохранение", "Медицинские расходы");
    createExpenseCategory("Развлечения", "Развлечения и отдых");
    createExpenseCategory("Покупки", "Общие покупки");
    createExpenseCategory("Образование", "Образовательные расходы");
    createExpenseCategory("Другой расход", "Другие расходы");
  }
};

// Фасад управления операциями
class OperationFacade {
 private:
  std::shared_ptr<IOperationRepository> operationRepo_;
  std::shared_ptr<OperationProcessingService> processingService_;
  std::shared_ptr<IEntityFactory> factory_;
  std::shared_ptr<CommandHistory> history_;
  int decorationFlags_;

 public:
  // В этом классе во избежание циклической зависимости между классами
  //  OperationProcessingService IOperationRepository и OperationFacade, мне
  //  пришлось сделать
  // ленивую инициализацию поля processingService_. Другого
  // решения я к сожалению не придумал :(
  OperationFacade(int decorationFlags = DecoratedCommandFactory::ALL)
      : decorationFlags_(decorationFlags) {
    operationRepo_ = ServiceLocator::get<IOperationRepository>();
    factory_ = ServiceLocator::get<IEntityFactory>();
    history_ = std::make_shared<CommandHistory>();
  }

  // Добавить операции
  std::shared_ptr<Operation> addIncome(const Id& accountId, double amount,
                                       const Id& categoryId,
                                       const std::string& description = "",
                                       const std::string& currency = "RUB") {
    auto command = std::make_shared<AddOperationCommand>(
        OperationType::INCOME, accountId, Money(amount, currency), categoryId,
        description);

    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);

    return command->getCreatedOperation();
  }

  std::shared_ptr<Operation> addExpense(const Id& accountId, double amount,
                                        const Id& categoryId,
                                        const std::string& description = "",
                                        const std::string& currency = "RUB") {
    auto command = std::make_shared<AddOperationCommand>(
        OperationType::EXPENSE, accountId, Money(amount, currency), categoryId,
        description);

    auto decoratedCommand =
        DecoratedCommandFactory::decorate(command, decorationFlags_);
    history_->execute(decoratedCommand);

    return command->getCreatedOperation();
  }

  // Быстрые операции с категориями по умолчанию
  std::shared_ptr<Operation> addQuickIncome(
      const Id& accountId, double amount, const std::string& description = "") {
    auto categoryFacade = CategoryFacade();
    auto category = categoryFacade.getCategoryByName("Другой доход");
    if (!category) {
      category = categoryFacade.createIncomeCategory("Другой доход",
                                                     "Другие источники дохода");
    }

    return addIncome(accountId, amount, category->getId(), description);
  }

  std::shared_ptr<Operation> addQuickExpense(
      const Id& accountId, double amount, const std::string& description = "") {
    auto categoryFacade = CategoryFacade();
    auto category = categoryFacade.getCategoryByName("Другой расход");
    if (!category) {
      category = categoryFacade.createExpenseCategory("Другой расход",
                                                      "Другие расходы");
    }

    return addExpense(accountId, amount, category->getId(), description);
  }

  // Запросы операций
  std::shared_ptr<Operation> getOperation(const Id& operationId) {
    auto result = operationRepo_->findById(operationId);
    return result ? *result : nullptr;
  }

  std::vector<std::shared_ptr<Operation>> getAccountOperations(
      const Id& accountId) {
    return operationRepo_->findByAccount(accountId);
  }

  std::vector<std::shared_ptr<Operation>> getCategoryOperations(
      const Id& categoryId) {
    return operationRepo_->findByCategory(categoryId);
  }

  std::vector<std::shared_ptr<Operation>> getOperationsByDateRange(
      const DateTime& start, const DateTime& end) {
    return operationRepo_->findByDateRange(start, end);
  }

  std::vector<std::shared_ptr<Operation>> getOperationsByType(
      OperationType type) {
    return operationRepo_->findByType(type);
  }

  std::vector<std::shared_ptr<Operation>> getTodayOperations() {
    auto range = DateRange::today();
    return operationRepo_->findByDateRange(range.getStart(), range.getEnd());
  }

  std::vector<std::shared_ptr<Operation>> getMonthOperations() {
    auto range = DateRange::thisMonth();
    return operationRepo_->findByDateRange(range.getStart(), range.getEnd());
  }

  // Управление операциями
  void updateOperation(const Id& operationId, const Money& newAmount,
                       const std::string& newDescription) {
    auto operation = getOperation(operationId);
    if (!operation) {
      throw EntityNotFoundException("Operation", operationId);
    }

    // Обновить операцию
    operation->setAmount(newAmount);
    operation->setDescription(newDescription);
    operationRepo_->update(operation);

    // Пересчитать баланс счёта
    auto reconciliationService =
        ServiceLocator::get<BalanceReconciliationService>();
    reconciliationService->recalculateBalance(operation->getBankAccountId(),
                                              true);
  }

  void deleteOperation(const Id& operationId) {
    auto operation = getOperation(operationId);
    if (!operation) {
      throw EntityNotFoundException("Operation", operationId);
    }

    auto accountId = operation->getBankAccountId();

    // Удалить операцию
    operationRepo_->remove(operationId);

    // Пересчитать баланс счёта
    auto reconciliationService =
        ServiceLocator::get<BalanceReconciliationService>();
    reconciliationService->recalculateBalance(accountId, true);
  }

  // Повторяющиеся операции
  void setRecurring(const Id& operationId, const std::string& pattern) {
    auto operation = getOperation(operationId);
    if (!operation) {
      throw EntityNotFoundException("Operation", operationId);
    }

    operation->setRecurring(true, pattern);
    operationRepo_->update(operation);
  }

  void processRecurringOperations() {
    if (!processingService_) {
      processingService_ = ServiceLocator::get<OperationProcessingService>();
    }
    processingService_->processRecurringOperations(DateTimeUtils::now());
  }

  // Управление историей
  void undo() { history_->undo(); }

  void redo() { history_->redo(); }

  bool canUndo() const { return history_->canUndo(); }

  bool canRedo() const { return history_->canRedo(); }
};

}  // namespace financial::application