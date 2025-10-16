#include <iostream>
#include "infrastructure/DIContainer.h"
#include "infrastructure/DIConfig.h"
#include "ui/ConsoleUI.h"

int main() {
  try {
    // Создаём и настраиваем DI-контейнер
    DIContainer container;
    DIConfig::configure(container);

    // Создаём ZooManager с внедрёнными зависимостями
    auto zooManager = DIConfig::createZooManager(container);

    // Создаём UI с внедрённым ZooManager
    ConsoleUI ui(zooManager);

    // Запускаем приложение
    ui.run();

  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}