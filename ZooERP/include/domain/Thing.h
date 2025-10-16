#ifndef THING_H
#define THING_H

#include <string>
#include "interfaces/IInventory.h"

/**
 * @brief Базовый класс для вещей на балансе зоопарка
 *
 * Применение SOLID:
 * - SRP: отвечает только за данные неживых объектов
 * - OCP: открыт для расширения (Table, Computer, и т.д.)
 * - ISP: реализует только IInventory (вещи не живые, не едят)
 */
class Thing : public IInventory {
protected:
  std::string name_;          // Название вещи
  int inventoryNumber_;       // Инвентаризационный номер

public:
  /**
   * @brief Конструктор вещи
   * @param name Название вещи
   * @param inventoryNumber Инвентаризационный номер (> 0)
   */
  Thing(const std::string& name, int inventoryNumber);

  virtual ~Thing() = default;

  // Реализация IInventory
  int getInventoryNumber() const override { return inventoryNumber_; }
  void setInventoryNumber(int number) override;

  // Дополнительные методы
  std::string getName() const { return name_; }
  void setName(const std::string& name) { name_ = name; }

  /**
   * @brief Получить информацию о вещи
   */
  virtual std::string getInfo() const;

  /**
   * @brief Получить тип вещи
   */
  virtual std::string getType() const = 0;
};

#endif // THING_H