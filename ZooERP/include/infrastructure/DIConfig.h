#ifndef DICONFIG_H
#define DICONFIG_H

#include "infrastructure/DIContainer.h"
#include "services/VeterinaryClinic.h"
#include "services/ZooRepository.h"
#include "services/ZooManager.h"
#include "interfaces/IVeterinaryClinic.h"
#include "interfaces/IZooRepository.h"

/**
 * @brief Конфигурация DI-контейнера
 *
 * Composition Root - единственное место, где создаются и связываются зависимости
 */
class DIConfig {
public:
  /**
   * @brief Настроить контейнер - зарегистрировать все сервисы
   * @param container DI-контейнер для настройки
   */
  static void configure(DIContainer& container) {
    // Регистрируем сервисы как Singleton
    // Один экземпляр на всё приложение

    // Ветеринарная клиника
    container.registerSingleton<IVeterinaryClinic, VeterinaryClinic>();

    // Репозиторий
    container.registerSingleton<IZooRepository, ZooRepository>();

    // ZooManager - с автоматическим разрешением зависимостей
    // Наш простой контейнер не умеет автоматически разрешать зависимости конструктора
    // Поэтому создадим фабрику вручную
  }

  /**
   * @brief Создать ZooManager с внедрёнными зависимостями
   * @param container DI-контейнер
   * @return Настроенный ZooManager
   */
  static std::shared_ptr<ZooManager> createZooManager(DIContainer& container) {
    // Разрешаем зависимости
    auto clinic = container.resolve<IVeterinaryClinic>();
    auto repository = container.resolve<IZooRepository>();

    // Создаём ZooManager с внедрением зависимостей через конструктор
    return std::make_shared<ZooManager>(clinic, repository);
  }
};

#endif // DICONFIG_H