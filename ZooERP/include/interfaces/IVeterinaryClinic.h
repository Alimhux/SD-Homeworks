#ifndef IVETERINARYCLINIC_H
#define IVETERINARYCLINIC_H

#include "interfaces/IHealthCheckable.h"

/**
 * @brief Интерфейс ветеринарной клиники
 *
 * Применение DIP: высокоуровневые модули (ZooManager)
 * зависят от этой абстракции, а не от конкретной реализации
 */
class IVeterinaryClinic {
 public:
  virtual ~IVeterinaryClinic() = default;

  /**
   * @brief Проверить здоровье животного
   * @param animal Животное для проверки
   * @return true если животное здорово и может быть принято в зоопарк
   */
  virtual bool checkHealth(const IHealthCheckable& animal) = 0;
};

#endif  // IVETERINARYCLINIC_H