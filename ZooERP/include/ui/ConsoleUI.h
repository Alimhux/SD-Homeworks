#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include <memory>
#include <string>
#include "services/ZooManager.h"

/**
 * @brief Консольный пользовательский интерфейс
 *
 * Применение SRP: отвечает только за взаимодействие с пользователем
 * Применение DIP: зависит от абстракции ZooManager
 */
class ConsoleUI {
private:
  std::shared_ptr<ZooManager> zooManager_;

  // Вспомогательные методы для меню
  void showMenu() const;
  void handleAddAnimal();
  void handleShowAllAnimals() const;
  void handleShowTotalFood() const;
  void handleShowInteractiveAnimals() const;
  void handleShowInventory() const;
  void addSampleData();  // Для быстрого тестирования

  // Утилиты для ввода
  int readInt(const std::string& prompt) const;
  std::string readString(const std::string& prompt) const;
  bool readBool(const std::string& prompt) const;

public:
  /**
   * @brief Конструктор с внедрением ZooManager
   */
  explicit ConsoleUI(std::shared_ptr<ZooManager> manager);

  /**
   * @brief Запустить главный цикл приложения
   */
  void run();
};

#endif // CONSOLEUI_H