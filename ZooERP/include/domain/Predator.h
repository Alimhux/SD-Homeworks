#ifndef PREDATOR_H
#define PREDATOR_H

#include "domain/Animal.h"

/**
 * @brief Класс хищных животных
 *
 * Особенности:
 * - Не могут быть интерактивными (для безопасности)
 * - Обычно потребляют больше мяса
 *
 * Применение SOLID:
 * - SRP: отвечает за специфику хищников
 * - LSP: полностью заменяем Animal
 * - ISP: НЕ реализует IInteractive (хищники не для контактного зоопарка)
 */
class Predator : public Animal {
public:
  /**
   * @brief Конструктор хищника
   */
  Predator(const std::string& name, int inventoryNumber, int food,
           bool isHealthy = true)
      : Animal(name, inventoryNumber, food, isHealthy) {}

  virtual ~Predator() = default;

  // Хищники не могут быть в контактном зоопарке
  // Поэтому IInteractive не реализуем
  // getType остаётся абстрактным - каждый хищник определит свой тип
  std::string getType() const override = 0;
};

#endif // PREDATOR_H