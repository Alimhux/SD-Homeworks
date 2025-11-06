#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <any>
#include <set>
#include <iostream>
#include "domain/repositories/repository_interfaces.h"
#include "domain/factories/entity_factory.h"
#include "domain/services/domain_services.h"
#include "infrastructure/persistence/in_memory_repository.h"
#include "infrastructure/proxy/caching_proxy.h"

namespace financial::infrastructure {

// DI-контейнер с защитой от циклических зависимостей
class DIContainer {
private:
    static std::unique_ptr<DIContainer> instance_;
    static std::mutex mutex_;

    // Реестр служб
    std::unordered_map<std::type_index, std::any> services_;
    std::unordered_map<std::type_index, std::function<std::any()>> factories_;
    mutable std::mutex servicesMutex_;

    // Для обнаружения циклических зависимостей
    std::set<std::type_index> resolvingTypes_;
    mutable std::mutex resolvingMutex_;

    DIContainer() = default;

public:
    DIContainer(const DIContainer&) = delete;
    DIContainer& operator=(const DIContainer&) = delete;

    static DIContainer& getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_ = std::unique_ptr<DIContainer>(new DIContainer());
        }
        return *instance_;
    }

    template<typename Interface, typename Implementation>
    void registerSingleton(std::shared_ptr<Implementation> implementation) {
        std::lock_guard<std::mutex> lock(servicesMutex_);
        services_[std::type_index(typeid(Interface))] =
            std::static_pointer_cast<Interface>(implementation);
    }

    template<typename Interface>
    void registerSingleton(std::function<std::shared_ptr<Interface>()> factory) {
        std::lock_guard<std::mutex> lock(servicesMutex_);
        auto service = factory();
        services_[std::type_index(typeid(Interface))] = service;
    }

    template<typename Interface>
    void registerTransient(std::function<std::shared_ptr<Interface>()> factory) {
        std::lock_guard<std::mutex> lock(servicesMutex_);
        factories_[std::type_index(typeid(Interface))] =
            [factory]() -> std::any { return factory(); };
    }

    template<typename Interface>
    std::shared_ptr<Interface> resolve() {
        std::type_index typeIdx = std::type_index(typeid(Interface));

        // Проверяем циклическую зависимость
        {
            std::lock_guard<std::mutex> lock(resolvingMutex_);
            if (resolvingTypes_.find(typeIdx) != resolvingTypes_.end()) {
                throw std::runtime_error("Циклическая зависимость при разрешении: " +
                    std::string(typeid(Interface).name()));
            }
            resolvingTypes_.insert(typeIdx);
        }

        std::lock_guard<std::mutex> lock(servicesMutex_);
        std::shared_ptr<Interface> result;

        try {
            auto it = services_.find(typeIdx);
            if (it != services_.end()) {
                result = std::any_cast<std::shared_ptr<Interface>>(it->second);
            } else {
                auto factoryIt = factories_.find(typeIdx);
                if (factoryIt != factories_.end()) {
                    result = std::any_cast<std::shared_ptr<Interface>>(factoryIt->second());
                } else {
                    throw std::runtime_error("Служба не зарегистрирована: " +
                        std::string(typeid(Interface).name()));
                }
            }
        } catch (const std::bad_any_cast& e) {
            {
                std::lock_guard<std::mutex> lock(resolvingMutex_);
                resolvingTypes_.erase(typeIdx);
            }
            throw std::runtime_error("Не удалось разрешить зависимость: несоответствие типов");
        }

        // Удаляем из стека разрешения
        {
            std::lock_guard<std::mutex> lock(resolvingMutex_);
            resolvingTypes_.erase(typeIdx);
        }

        return result;
    }

    template<typename Interface>
    bool isRegistered() const {
        std::lock_guard<std::mutex> lock(servicesMutex_);
        auto typeIdx = std::type_index(typeid(Interface));
        return services_.count(typeIdx) > 0 || factories_.count(typeIdx) > 0;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(servicesMutex_);
        services_.clear();
        factories_.clear();
    }

    static void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_.reset();
    }
};

inline std::unique_ptr<DIContainer> DIContainer::instance_ = nullptr;
inline std::mutex DIContainer::mutex_;

class ServiceConfigurator {
public:
    static void configureServices(bool useCaching = true) {
        auto& container = DIContainer::getInstance();
        container.clear();

        // 1. Сначала регистрируем фабрики и UnitOfWork (базовые зависимости)
        container.registerSingleton<domain::IEntityFactory>(
            []() { return std::make_shared<domain::EntityFactory>(); });

        container.registerSingleton<domain::IUnitOfWork>(
            []() { return std::make_shared<InMemoryUnitOfWork>(); });

        auto unitOfWork = container.resolve<domain::IUnitOfWork>();

        // 2. Затем регистрируем репозитории (зависят от UnitOfWork, но не от сервисов)
        container.registerSingleton<domain::IBankAccountRepository>(
            [unitOfWork, useCaching]() -> std::shared_ptr<domain::IBankAccountRepository> {
                auto repo = std::make_shared<InMemoryBankAccountRepository>();
                if (useCaching) {
                    return CachingProxyFactory::createCachingBankAccountRepository(
                        repo, std::chrono::seconds(60));
                }
                return repo;
            });

        container.registerSingleton<domain::ICategoryRepository>(
            [unitOfWork]() -> std::shared_ptr<domain::ICategoryRepository> {
                return std::make_shared<InMemoryCategoryRepository>();
            });

        container.registerSingleton<domain::IOperationRepository>(
            [unitOfWork]() -> std::shared_ptr<domain::IOperationRepository> {
                return std::make_shared<InMemoryOperationRepository>();
            });

        // 3. И только потом регистрируем сервисы (зависят от репозиториев)
        container.registerTransient<domain::AnalyticsService>(
            []() {
                auto& c = DIContainer::getInstance();
                return std::make_shared<domain::AnalyticsService>(
                    c.resolve<domain::IOperationRepository>(),
                    c.resolve<domain::ICategoryRepository>()
                );
            });

        container.registerTransient<domain::BalanceReconciliationService>(
            []() {
                auto& c = DIContainer::getInstance();
                return std::make_shared<domain::BalanceReconciliationService>(
                    c.resolve<domain::IBankAccountRepository>(),
                    c.resolve<domain::IOperationRepository>()
                );
            });

        container.registerTransient<domain::OperationProcessingService>(
            []() {
                auto& c = DIContainer::getInstance();
                return std::make_shared<domain::OperationProcessingService>(
                    c.resolve<domain::IBankAccountRepository>(),
                    c.resolve<domain::IOperationRepository>(),
                    c.resolve<domain::IEntityFactory>()
                );
            });
    }

    static void configureTestServices() {
        configureServices(false);
    }
};

class ServiceLocator {
public:
    template<typename T>
    static std::shared_ptr<T> get() {
        return DIContainer::getInstance().resolve<T>();
    }

    template<typename T>
    static bool has() {
        return DIContainer::getInstance().isRegistered<T>();
    }
};

} // namespace financial::infrastructure