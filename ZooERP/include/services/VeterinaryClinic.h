#ifndef VETERINARYCLINIC_H
#define VETERINARYCLINIC_H

#include "interfaces/IVeterinaryClinic.h"

/**
 * @brief Реализация ветеринарной клиники
 *
 * Применение SRP: отвечает только за проверку здоровья животных
 */
class VeterinaryClinic : public IVeterinaryClinic {
public:
  VeterinaryClinic() = default;
  virtual ~VeterinaryClinic() = default;

  /**
   * @brief Проверяет здоровье животного
   * @param animal Животное для проверки
   * @return true если животное здорово
   */
  bool checkHealth(const IHealthCheckable& animal) override;
};

#endif // VETERINARYCLINIC_H