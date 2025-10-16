#ifndef DICONTAINER_H
#define DICONTAINER_H

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <typeinfo>
#include <stdexcept>

/**
 * Простой DI-контейнер для управления зависимостями
 *
 * Поддерживает:
 * - Singleton: один экземпляр на всё приложение
 * - Transient: новый экземпляр при каждом запросе
 *
 * Применение паттерна Service Locator ТОЛЬКО внутри контейнера
 */
class DIContainer {
private:
  // фабрики для создания объектов.
  std::map<std::string, std::function<std::shared_ptr<void>()>> factories_;

  // синглтоны (кеш созданных объектов)
  std::map<std::string, std::shared_ptr<void>> singletons_;

  // флаги: является ли тип синглтоном.
  std::map<std::string, bool> isSingleton_;

public:
  template<typename TInterface, typename TImplementation, typename... Args>
  void registerSingleton(Args&&... args) {
    std::string key = typeid(TInterface).name();

    isSingleton_[key] = true;

    factories_[key] = [args...]() -> std::shared_ptr<void> {
      return std::static_pointer_cast<void>(
        std::make_shared<TImplementation>(args...)
        );
    };
  }

  /**
     * @brief Регистрирует тип как Transient
     * Новый экземпляр при каждом вызове resolve()
     */
  template<typename TInterface, typename TImplementation, typename... Args>
  void registerTransient(Args&&... args) {
    std::string key = typeid(TInterface).name();

    isSingleton_[key] = false;

    factories_[key] = [args...]() -> std::shared_ptr<void> {
      return std::static_pointer_cast<void>(
          std::make_shared<TImplementation>(args...)
      );
    };
  }

  /**
     * @brief Разрешает зависимость (получает экземпляр)
     * @return Экземпляр зарегистрированного типа
     * @throws std::runtime_error если тип не зарегистрирован
     */
  template<typename T>
  std::shared_ptr<T> resolve() {
    std::string key = typeid(T).name();

    // Проверяем, что тип зарегистрирован
    if (factories_.find(key) == factories_.end()) {
      throw std::runtime_error(
          std::string("Type not registered in DI container: ") + key
      );
    }

    // Если это синглтон и уже создан - возвращаем существующий
    if (isSingleton_[key] && singletons_.find(key) != singletons_.end()) {
      return std::static_pointer_cast<T>(singletons_[key]);
    }

    // Создаём новый экземпляр через фабрику
    auto instance = std::static_pointer_cast<T>(factories_[key]());

    // Сохраняем синглтон в кеш
    if (isSingleton_[key]) {
      singletons_[key] = instance;
    }

    return instance;
  }

  /**
    * @brief Очищает все синглтоны (для тестов)
    */
  void clear() {
    singletons_.clear();
  }
};

#endif //DICONTAINER_H
