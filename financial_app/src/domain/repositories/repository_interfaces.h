#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "common/types.h"
#include "domain/value_objects/types.h"

namespace financial::domain {

class BankAccount;
class Category;
class Operation;

// Общий интерфейс для репозиториев
template <typename T>
class IRepository {
 public:
  virtual ~IRepository() = default;

  virtual void save(std::shared_ptr<T> entity) = 0;
  virtual void update(std::shared_ptr<T> entity) = 0;
  virtual void remove(const Id& id) = 0;
  virtual std::optional<std::shared_ptr<T>> findById(const Id& id) = 0;
  virtual std::vector<std::shared_ptr<T>> findAll() = 0;
  virtual size_t count() = 0;
  virtual void clear() = 0;
};

// Интерфейс репозиторев для банковских счетов
class IBankAccountRepository : public virtual IRepository<BankAccount> {
 public:
  virtual std::vector<std::shared_ptr<BankAccount>> findActive() = 0;
  virtual std::optional<std::shared_ptr<BankAccount>> findByAccountNumber(
      const std::string& accountNumber) = 0;
};

// Интерфейс репозиторев для категорий трат
class ICategoryRepository : public virtual IRepository<Category> {
 public:
  virtual std::vector<std::shared_ptr<Category>> findByType(
      CategoryType type) = 0;
  virtual std::optional<std::shared_ptr<Category>> findByName(
      const std::string& name) = 0;
};

// Интерфейс репозиторев для банковский операций
class IOperationRepository : public virtual IRepository<Operation> {
 public:
  virtual std::vector<std::shared_ptr<Operation>> findByAccount(
      const Id& accountId) = 0;
  virtual std::vector<std::shared_ptr<Operation>> findByCategory(
      const Id& categoryId) = 0;
  virtual std::vector<std::shared_ptr<Operation>> findByDateRange(
      const DateTime& start, const DateTime& end) = 0;
  virtual std::vector<std::shared_ptr<Operation>> findByType(
      OperationType type) = 0;

  // Поиск по какому-то условию
  virtual std::vector<std::shared_ptr<Operation>> findWhere(
      std::function<bool(const Operation&)> predicate) = 0;
};

// паттерн Unit of Work для реализации операций
class IUnitOfWork {
 public:
  virtual ~IUnitOfWork() = default;

  virtual void begin() = 0;
  virtual void commit() = 0;
  virtual void rollback() = 0;

  virtual IBankAccountRepository& accounts() = 0;
  virtual ICategoryRepository& categories() = 0;
  virtual IOperationRepository& operations() = 0;
};

}  // namespace financial::domain