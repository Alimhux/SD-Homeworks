#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "domain/entities/bank_account.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"
#include "domain/repositories/repository_interfaces.h"

namespace financial::infrastructure {

using namespace financial::domain;

// Защищённый от многопоточных запросов репозиторий
template <typename T>
class InMemoryRepository : public IRepository<T> {
 protected:
  mutable std::mutex mutex_;
  std::unordered_map<Id, std::shared_ptr<T>> storage_;

 public:
  void save(std::shared_ptr<T> entity) override {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_[entity->getId()] = entity;
  }

  void update(std::shared_ptr<T> entity) override {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = storage_.find(entity->getId());
    if (it == storage_.end()) {
      throw EntityNotFoundException("Entity", entity->getId());
    }
    storage_[entity->getId()] = entity;
  }

  void remove(const Id& id) override {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_.erase(id);
  }

  std::optional<std::shared_ptr<T>> findById(const Id& id) override {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = storage_.find(id);
    if (it != storage_.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  std::vector<std::shared_ptr<T>> findAll() override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<T>> result;
    result.reserve(storage_.size());

    for (const auto& [id, entity] : storage_) {
      result.push_back(entity);
    }

    return result;
  }

  size_t count() override {
    std::lock_guard<std::mutex> lock(mutex_);
    return storage_.size();
  }

  void clear() override {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_.clear();
  }
};

// BankAccount репозиторий
class InMemoryBankAccountRepository : public InMemoryRepository<BankAccount>,
                                      virtual public IBankAccountRepository {
 public:
  void save(std::shared_ptr<BankAccount> entity) override {
    InMemoryRepository<BankAccount>::save(entity);
  }

  void update(std::shared_ptr<BankAccount> entity) override {
    InMemoryRepository<BankAccount>::update(entity);
  }

  void remove(const Id& id) override {
    InMemoryRepository<BankAccount>::remove(id);
  }

  std::optional<std::shared_ptr<BankAccount>> findById(const Id& id) override {
    return InMemoryRepository<BankAccount>::findById(id);
  }

  std::vector<std::shared_ptr<BankAccount>> findAll() override {
    return InMemoryRepository<BankAccount>::findAll();
  }

  size_t count() override {
    return InMemoryRepository<BankAccount>::count();
  }

  void clear() override {
    InMemoryRepository<BankAccount>::clear();
  }

  std::vector<std::shared_ptr<BankAccount>> findActive() override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<BankAccount>> result;

    for (const auto& [id, account] : storage_) {
      if (account->getIsActive()) {
        result.push_back(account);
      }
    }

    return result;
  }

  std::optional<std::shared_ptr<BankAccount>> findByAccountNumber(
      const std::string& accountNumber) override {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [id, account] : storage_) {
      if (account->getAccountNumber() == accountNumber) {
        return account;
      }
    }

    return std::nullopt;
  }
};

// Category репозиторий
class InMemoryCategoryRepository : public InMemoryRepository<Category>,
                                   virtual public ICategoryRepository {

 public:
  void save(std::shared_ptr<Category> entity) override {
    InMemoryRepository<Category>::save(entity);
  }

  void update(std::shared_ptr<Category> entity) override {
    InMemoryRepository<Category>::update(entity);
  }

  void remove(const Id& id) override {
    InMemoryRepository<Category>::remove(id);
  }

  std::optional<std::shared_ptr<Category>> findById(const Id& id) override {
    return InMemoryRepository<Category>::findById(id);
  }

  std::vector<std::shared_ptr<Category>> findAll() override {
    return InMemoryRepository<Category>::findAll();
  }

  size_t count() override {
    return InMemoryRepository<Category>::count();
  }

  void clear() override {
    InMemoryRepository<Category>::clear();
  }

  std::vector<std::shared_ptr<Category>> findByType(
      CategoryType type) override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Category>> result;

    for (const auto& [id, category] : storage_) {
      if (category->getType() == type) {
        result.push_back(category);
      }
    }

    return result;
  }

  std::optional<std::shared_ptr<Category>> findByName(
      const std::string& name) override {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [id, category] : storage_) {
      if (category->getName() == name) {
        return category;
      }
    }

    return std::nullopt;
  }
};

// Репозиторий операций
class InMemoryOperationRepository : public InMemoryRepository<Operation>,
                                    virtual public IOperationRepository {
 public:

  void save(std::shared_ptr<Operation> entity) override {
    InMemoryRepository<Operation>::save(entity);
  }

  void update(std::shared_ptr<Operation> entity) override {
    InMemoryRepository<Operation>::update(entity);
  }

  void remove(const Id& id) override {
    InMemoryRepository<Operation>::remove(id);
  }

  std::optional<std::shared_ptr<Operation>> findById(const Id& id) override {
    return InMemoryRepository<Operation>::findById(id);
  }

  std::vector<std::shared_ptr<Operation>> findAll() override {
    return InMemoryRepository<Operation>::findAll();
  }

  size_t count() override {
    return InMemoryRepository<Operation>::count();
  }

  void clear() override {
    InMemoryRepository<Operation>::clear();
  }

  std::vector<std::shared_ptr<Operation>> findByAccount(
      const Id& accountId) override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Operation>> result;

    for (const auto& [id, operation] : storage_) {
      if (operation->getBankAccountId() == accountId) {
        result.push_back(operation);
      }
    }

    // Сортируем по дате
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
      return a->getDate() > b->getDate();
    });

    return result;
  }

  std::vector<std::shared_ptr<Operation>> findByCategory(
      const Id& categoryId) override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Operation>> result;

    for (const auto& [id, operation] : storage_) {
      if (operation->getCategoryId() == categoryId) {
        result.push_back(operation);
      }
    }

    return result;
  }

  std::vector<std::shared_ptr<Operation>> findByDateRange(
      const DateTime& start, const DateTime& end) override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Operation>> result;
    DateRange range(start, end);

    for (const auto& [id, operation] : storage_) {
      if (operation->isInDateRange(range)) {
        result.push_back(operation);
      }
    }

    // Сортируем по дате
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
      return a->getDate() > b->getDate();
    });

    return result;
  }

  std::vector<std::shared_ptr<Operation>> findByType(
      OperationType type) override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Operation>> result;

    for (const auto& [id, operation] : storage_) {
      if (operation->getType() == type) {
        result.push_back(operation);
      }
    }

    return result;
  }

  std::vector<std::shared_ptr<Operation>> findWhere(
      std::function<bool(const Operation&)> predicate) override {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Operation>> result;

    for (const auto& [id, operation] : storage_) {
      if (predicate(*operation)) {
        result.push_back(operation);
      }
    }

    return result;
  }
};

// реализация Unit of Workа для транзакций
class InMemoryUnitOfWork : public IUnitOfWork {
 private:
  std::shared_ptr<IBankAccountRepository> accountRepo_;
  std::shared_ptr<ICategoryRepository> categoryRepo_;
  std::shared_ptr<IOperationRepository> operationRepo_;

  // отслеживает состояние транзакции
  bool inTransaction_ = false;

 public:
  InMemoryUnitOfWork() {
    accountRepo_ = std::make_shared<InMemoryBankAccountRepository>();
    categoryRepo_ = std::make_shared<InMemoryCategoryRepository>();
    operationRepo_ = std::make_shared<InMemoryOperationRepository>();
  }

  void begin() override { inTransaction_ = true; }

  void commit() override { inTransaction_ = false; }

  void rollback() override { inTransaction_ = false; }

  IBankAccountRepository& accounts() override { return *accountRepo_; }

  ICategoryRepository& categories() override { return *categoryRepo_; }

  IOperationRepository& operations() override { return *operationRepo_; }
};

}  // namespace financial::infrastructure