#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <any>
#include "domain/repositories/repository_interfaces.h"
#include "domain/factories/entity_factory.h"
#include "domain/services/domain_services.h"
#include "infrastructure/persistence/in_memory_repository.h"
#include "infrastructure/proxy/caching_proxy.h"

namespace financial::infrastructure {

// Упрощённый DI-контейнер
class DIContainer {
private:
    // Экземпляр Singleton
    static std::unique_ptr<DIContainer> instance_;
    static std::mutex mutex_;

    // Реестр сервисов
    std::unordered_map<std::type_index, std::any> services_;
    std::unordered_map<std::type_index, std::function<std::any()>> factories_;
    mutable std::recursive_mutex servicesMutex_;

    DIContainer() = default;

public:
    // Удаляем конструктор копирования и оператор присваивания
    DIContainer(const DIContainer&) = delete;
    DIContainer& operator=(const DIContainer&) = delete;

    // Получить экземпляр singleton
    static DIContainer& getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_ = std::unique_ptr<DIContainer>(new DIContainer());
        }
        return *instance_;
    }

    // Зарегистрировать singleton-сервис
    template<typename Interface, typename Implementation>
    void registerSingleton(std::shared_ptr<Implementation> implementation) {
        std::lock_guard<std::recursive_mutex> lock(servicesMutex_);
        services_[std::type_index(typeid(Interface))] =
            std::static_pointer_cast<Interface>(implementation);
    }

    // Зарегистрировать singleton-сервис через фабрику
    template<typename Interface>
    void registerSingleton(std::function<std::shared_ptr<Interface>()> factory) {
        std::lock_guard<std::recursive_mutex> lock(servicesMutex_);
        auto service = factory();
        services_[std::type_index(typeid(Interface))] = service;
    }

    // Зарегистрировать transient-сервис (новый экземпляр при каждом вызове)
    template<typename Interface>
    void registerTransient(std::function<std::shared_ptr<Interface>()> factory) {
        std::lock_guard<std::recursive_mutex> lock(servicesMutex_);
        factories_[std::type_index(typeid(Interface))] =
            [factory]() -> std::any { return factory(); };
    }

    // Зарезолвить сервис
    template<typename Interface>
    std::shared_ptr<Interface> resolve() {
        std::lock_guard<std::recursive_mutex> lock(servicesMutex_);

        // Сначала проверяем на наличие singleton
        auto it = services_.find(std::type_index(typeid(Interface)));
        if (it != services_.end()) {
            try {
                return std::any_cast<std::shared_ptr<Interface>>(it->second);
            } catch (const std::bad_any_cast& e) {
                throw std::runtime_error("Не удалось разрешить сервис: несоответствие типов");
            }
        }

        // Проверяем фабрику
        auto factoryIt = factories_.find(std::type_index(typeid(Interface)));
        if (factoryIt != factories_.end()) {
            try {
                return std::any_cast<std::shared_ptr<Interface>>(factoryIt->second());
            } catch (const std::bad_any_cast& e) {
                throw std::runtime_error("Не удалось разрешить сервис из фабрики: несоответствие типов");
            }
        }

        throw std::runtime_error("Сервис не зарегистрирован: " +
            std::string(typeid(Interface).name()));
    }

    // Проверить, зарегистрирован ли сервис
    template<typename Interface>
    bool isRegistered() const {
        std::lock_guard<std::recursive_mutex> lock(servicesMutex_);
        auto typeIdx = std::type_index(typeid(Interface));
        return services_.count(typeIdx) > 0 || factories_.count(typeIdx) > 0;
    }

    // Очистить все регистрации (полезно для тестирования)
    void clear() {
        std::lock_guard<std::recursive_mutex> lock(servicesMutex_);
        services_.clear();
        factories_.clear();
    }

    // Сбросить экземпляр singleton (полезно для тестирования)
    static void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_.reset();
    }
};

// Определения статических членов
inline std::unique_ptr<DIContainer> DIContainer::instance_ = nullptr;
inline std::mutex DIContainer::mutex_;

// Конфигуратор сервисов для упрощённой настройки DI
class ServiceConfigurator {
public:
    static void configureServices(bool useCaching = true) {
        auto& container = DIContainer::getInstance();

        container.clear();

        container.registerSingleton<domain::IEntityFactory>(
            []() { return std::make_shared<domain::EntityFactory>(); });

        container.registerSingleton<domain::IUnitOfWork>(
            []() { return std::make_shared<InMemoryUnitOfWork>(); });

        auto unitOfWork = container.resolve<domain::IUnitOfWork>();

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

        // Зарегистрировать доменные сервисы
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

    // Конфигурация сервисов для тестов (с моками или тестовыми двойниками)
    static void configureTestServices() {
        configureServices(false);
    }
};

// Обёртка паттерна Service Locator для удобного доступа
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
