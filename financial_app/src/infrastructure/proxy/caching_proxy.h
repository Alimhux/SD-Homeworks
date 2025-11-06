#pragma once

#include <memory>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include "domain/repositories/repository_interfaces.h"

namespace financial::infrastructure {

using namespace financial::domain;

// Cache entry with expiration
template<typename T>
struct CacheEntry {
    std::shared_ptr<T> data;
    std::chrono::steady_clock::time_point expiry;
    
    bool isExpired() const {
        return std::chrono::steady_clock::now() > expiry;
    }
};

// Proxy pattern for caching repository access
template<typename T>
class CachingRepositoryProxy : public IRepository<T> {
private:
    std::shared_ptr<IRepository<T>> realRepository_;
    mutable std::unordered_map<Id, CacheEntry<T>> cache_;
    mutable std::mutex cacheMutex_;
    std::chrono::seconds cacheDuration_;
    
    void invalidateCache() {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        cache_.clear();
    }
    
    void cacheEntity(std::shared_ptr<T> entity) const {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        cache_[entity->getId()] = {
            entity,
            std::chrono::steady_clock::now() + cacheDuration_
        };
    }
    
public:
    CachingRepositoryProxy(
        std::shared_ptr<IRepository<T>> realRepository,
        std::chrono::seconds cacheDuration = std::chrono::seconds(60))
        : realRepository_(realRepository), cacheDuration_(cacheDuration) {}
    
    void save(std::shared_ptr<T> entity) override {
        realRepository_->save(entity);
        cacheEntity(entity);
    }
    
    void update(std::shared_ptr<T> entity) override {
        realRepository_->update(entity);
        cacheEntity(entity);
    }
    
    void remove(const Id& id) override {
        realRepository_->remove(id);
        
        std::lock_guard<std::mutex> lock(cacheMutex_);
        cache_.erase(id);
    }
    
    std::optional<std::shared_ptr<T>> findById(const Id& id) override {
        // Check cache first
        {
            std::lock_guard<std::mutex> lock(cacheMutex_);
            auto it = cache_.find(id);
            if (it != cache_.end() && !it->second.isExpired()) {
                return it->second.data;
            }
        }
        
        // Cache miss - fetch from real repository
        auto result = realRepository_->findById(id);
        if (result) {
            cacheEntity(*result);
        }
        
        return result;
    }
    
    std::vector<std::shared_ptr<T>> findAll() override {
        // For findAll, we don't use cache to ensure consistency
        auto results = realRepository_->findAll();
        
        // Update cache with fetched entities
        for (const auto& entity : results) {
            cacheEntity(entity);
        }
        
        return results;
    }
    
    size_t count() override {
        return realRepository_->count();
    }
    
    void clear() override {
        realRepository_->clear();
        invalidateCache();
    }
    
    // Cache management methods
    void clearCache() {
        invalidateCache();
    }
    
    size_t getCacheSize() const {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        return cache_.size();
    }
    
    void setCacheDuration(std::chrono::seconds duration) {
        cacheDuration_ = duration;
        invalidateCache();
    }
};

// Specialized caching proxy for BankAccount
class CachingBankAccountRepository : public IBankAccountRepository {
private:
    std::shared_ptr<IBankAccountRepository> realRepository_;
    std::shared_ptr<CachingRepositoryProxy<BankAccount>> cacheProxy_;
    
public:
    CachingBankAccountRepository(
        std::shared_ptr<IBankAccountRepository> realRepository,
        std::chrono::seconds cacheDuration = std::chrono::seconds(60))
        : realRepository_(realRepository) {
        cacheProxy_ = std::make_shared<CachingRepositoryProxy<BankAccount>>(
            realRepository, cacheDuration);
    }
    
    // Delegated methods from IRepository
    void save(std::shared_ptr<BankAccount> entity) override {
        cacheProxy_->save(entity);
    }
    
    void update(std::shared_ptr<BankAccount> entity) override {
        cacheProxy_->update(entity);
    }
    
    void remove(const Id& id) override {
        cacheProxy_->remove(id);
    }
    
    std::optional<std::shared_ptr<BankAccount>> findById(const Id& id) override {
        return cacheProxy_->findById(id);
    }
    
    std::vector<std::shared_ptr<BankAccount>> findAll() override {
        return cacheProxy_->findAll();
    }
    
    size_t count() override {
        return cacheProxy_->count();
    }
    
    void clear() override {
        cacheProxy_->clear();
    }
    
    // Specialized methods
    std::vector<std::shared_ptr<BankAccount>> findActive() override {
        // These methods go directly to the real repository
        // to ensure data consistency
        return realRepository_->findActive();
    }
    
    std::optional<std::shared_ptr<BankAccount>> findByAccountNumber(
        const std::string& accountNumber) override {
        return realRepository_->findByAccountNumber(accountNumber);
    }
};

// Factory for creating caching proxies
class CachingProxyFactory {
public:
    static std::shared_ptr<IBankAccountRepository> createCachingBankAccountRepository(
        std::shared_ptr<IBankAccountRepository> realRepository,
        std::chrono::seconds cacheDuration = std::chrono::seconds(60)) {
        return std::make_shared<CachingBankAccountRepository>(realRepository, cacheDuration);
    }

    static std::shared_ptr<ICategoryRepository> createCachingCategoryRepository(
        std::shared_ptr<ICategoryRepository> realRepository) {
        // Similar implementation for Category
        return realRepository; // Simplified for now
    }

    static std::shared_ptr<IOperationRepository> createCachingOperationRepository(
        std::shared_ptr<IOperationRepository> realRepository) {
        // Operations change more frequently, shorter cache duration
        return realRepository; // Simplified for now
    }
};

} // namespace financial::infrastructure