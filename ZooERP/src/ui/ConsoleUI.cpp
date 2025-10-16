#include "ui/ConsoleUI.h"

#include <iomanip>
#include <iostream>
#include <limits>

#include "domain/Computer.h"
#include "domain/Monkey.h"
#include "domain/Rabbit.h"
#include "domain/Table.h"
#include "domain/Tiger.h"
#include "domain/Wolf.h"

ConsoleUI::ConsoleUI(std::shared_ptr<ZooManager> manager)
    : zooManager_(manager) {
  if (!manager) {
    throw std::invalid_argument("ZooManager cannot be null");
  }
}

void ConsoleUI::run() {
  std::cout << "\n";
  std::cout << "===================================================\n";
  std::cout << "   МОСКОВСКИЙ ЗООПАРК - ERP СИСТЕМА                \n";
  std::cout << "   Система управления и учета зоопарка             \n";
  std::cout << "===================================================\n";
  std::cout << "\n";

  bool running = true;

  while (running) {
    showMenu();

    int choice = readInt("Введите ваш выбор: ");
    std::cout << "\n";

    try {
      switch (choice) {
        case 1:
          handleAddAnimal();
          break;
        case 2:
          handleShowAllAnimals();
          break;
        case 3:
          handleShowTotalFood();
          break;
        case 4:
          handleShowInteractiveAnimals();
          break;
        case 5:
          handleShowInventory();
          break;
        case 6:
          addSampleData();
          break;
        case 7:
          std::cout << "Спасибо за использование системы Московского зоопарка!\n";
          std::cout << "До свидания!\n\n";
          running = false;
          break;
        default:
          std::cout << "[!] Неверный выбор. Попробуйте снова.\n\n";
      }
    } catch (const std::exception& e) {
      std::cout << "[!] Ошибка: " << e.what() << "\n\n";
    }
  }
}

void ConsoleUI::showMenu() const {
  std::cout << "===================================================\n";
  std::cout << "  ГЛАВНОЕ МЕНЮ\n";
  std::cout << "===================================================\n";
  std::cout << "  1. Добавить новое животное\n";
  std::cout << "  2. Показать всех животных\n";
  std::cout << "  3. Рассчитать общее потребление корма\n";
  std::cout << "  4. Показать животных для контактного зоопарка\n";
  std::cout << "  5. Показать инвентарные номера\n";
  std::cout << "  6. Добавить тестовые данные\n";
  std::cout << "  7. Выход\n";
  std::cout << "===================================================\n";
}

void ConsoleUI::handleAddAnimal() {
  std::cout << "=== ДОБАВЛЕНИЕ НОВОГО ЖИВОТНОГО ===\n\n";

  std::cout << "Выберите тип животного:\n";
  std::cout << "  1. Кролик\n";
  std::cout << "  2. Обезьяна\n";
  std::cout << "  3. Тигр\n";
  std::cout << "  4. Волк\n";

  int type = readInt("Введите тип (1-4): ");

  std::string name = readString("Введите имя: ");
  int inventoryNumber = readInt("Введите инвентарный номер: ");
  int food = readInt("Введите количество корма в день (кг): ");
  bool isHealthy = readBool("Животное здорово? (1=да, 0=нет): ");

  std::shared_ptr<Animal> animal;

  try {
    switch (type) {
      case 1: {  // Rabbit
        int kindness = readInt("Введите уровень доброты (0-10): ");
        animal = std::make_shared<Rabbit>(name, inventoryNumber, food, kindness,
                                          isHealthy);
        break;
      }
      case 2: {  // Monkey
        int kindness = readInt("Введите уровень доброты (0-10): ");
        animal = std::make_shared<Monkey>(name, inventoryNumber, food, kindness,
                                          isHealthy);
        break;
      }
      case 3:  // Tiger
        animal =
            std::make_shared<Tiger>(name, inventoryNumber, food, isHealthy);
        break;
      case 4:  // Wolf
        animal = std::make_shared<Wolf>(name, inventoryNumber, food, isHealthy);
        break;
      default:
        std::cout << "[!] Неверный тип животного.\n\n";
        return;
    }

    // Добавляем через ZooManager (с проверкой здоровья)
    bool added = zooManager_->addAnimal(animal);

    if (added) {
      std::cout << "\n[+] Животное успешно добавлено в зоопарк!\n\n";
    } else {
      std::cout << "\n[-] Животное отклонено из-за проблем со здоровьем.\n\n";
    }

  } catch (const std::exception& e) {
    std::cout << "[!] Ошибка при создании животного: " << e.what() << "\n\n";
  }
}

void ConsoleUI::handleShowAllAnimals() const {
  std::cout << "=== ВСЕ ЖИВОТНЫЕ В ЗООПАРКЕ ===\n\n";

  auto animals = zooManager_->getAllAnimals();

  if (animals.empty()) {
    std::cout << "В зоопарке пока нет животных.\n\n";
    return;
  }

  std::cout << "Всего животных: " << animals.size() << "\n\n";

  // Таблица
  std::cout << std::left;
  std::cout << std::setw(20) << "Имя" << std::setw(12) << "Тип"
            << std::setw(10) << "Инв. №" << std::setw(12) << "Корм (кг)"
            << std::setw(15) << "Здоровье"
            << "\n";
  std::cout << std::string(69, '-') << "\n";

  for (const auto& animal : animals) {
    std::cout << std::setw(20) << animal->getName() << std::setw(12)
              << animal->getType() << std::setw(10)
              << animal->getInventoryNumber() << std::setw(12)
              << animal->getFood() << std::setw(15)
              << (animal->isHealthy() ? "Здоров" : "Болен") << "\n";
  }

  std::cout << "\n";
}

void ConsoleUI::handleShowTotalFood() const {
  std::cout << "=== ОТЧЕТ ПО ПОТРЕБЛЕНИЮ КОРМА ===\n\n";

  int totalFood = zooManager_->calculateTotalFood();
  size_t animalCount = zooManager_->getAnimalCount();

  std::cout << "Всего животных: " << animalCount << "\n";
  std::cout << "Общее потребление корма в день: " << totalFood << " кг\n";

  if (animalCount > 0) {
    double avgFood = static_cast<double>(totalFood) / animalCount;
    std::cout << "Среднее на одно животное: " << std::fixed << std::setprecision(2)
              << avgFood << " кг\n";
  }

  std::cout << "\n";
}

void ConsoleUI::handleShowInteractiveAnimals() const {
  std::cout << "=== ЖИВОТНЫЕ ДЛЯ КОНТАКТНОГО ЗООПАРКА ===\n\n";

  auto interactiveAnimals = zooManager_->getInteractiveAnimals();

  if (interactiveAnimals.empty()) {
    std::cout << "Нет животных для контактного зоопарка.\n";
    std::cout << "(Допускаются только травоядные с уровнем доброты >= 5)\n\n";
    return;
  }

  std::cout << "Животных, подходящих для контактного зоопарка: "
            << interactiveAnimals.size() << "\n\n";

  for (const auto& animal : interactiveAnimals) {
    std::cout << "  - " << animal->getName() << " (" << animal->getType() << ")"
              << " - Инв. №" << animal->getInventoryNumber() << "\n";
  }

  std::cout << "\n";
}

void ConsoleUI::handleShowInventory() const {
  std::cout << "=== ИНВЕНТАРИЗАЦИОННЫЙ ОТЧЕТ ===\n\n";

  auto animals = zooManager_->getAllAnimals();
  auto things = zooManager_->getAllThings();

  std::cout << "--- ЖИВОТНЫЕ ---\n";
  if (animals.empty()) {
    std::cout << "  (нет)\n";
  } else {
    for (const auto& animal : animals) {
      std::cout << "  Инв. №" << std::setw(5) << animal->getInventoryNumber()
                << " - " << animal->getName() << " (" << animal->getType()
                << ")\n";
    }
  }

  std::cout << "\n--- ВЕЩИ ---\n";
  if (things.empty()) {
    std::cout << "  (нет)\n";
  } else {
    for (const auto& thing : things) {
      std::cout << "  Инв. №" << std::setw(5) << thing->getInventoryNumber()
                << " - " << thing->getName() << " (" << thing->getType()
                << ")\n";
    }
  }

  std::cout << "\nВсего инвентарных объектов: "
            << (animals.size() + things.size()) << "\n\n";
}

void ConsoleUI::addSampleData() {
  std::cout << "=== ДОБАВЛЕНИЕ ТЕСТОВЫХ ДАННЫХ ===\n\n";

  try {
    // Добавляем животных
    zooManager_->addAnimal(std::make_shared<Rabbit>("Пушистик", 101, 2, 8, true));
    zooManager_->addAnimal(
        std::make_shared<Rabbit>("Прыгун", 102, 2, 6, true));
    zooManager_->addAnimal(std::make_shared<Monkey>("Джордж", 103, 5, 9, true));
    zooManager_->addAnimal(
        std::make_shared<Tiger>("Шерхан", 104, 18, true));
    zooManager_->addAnimal(std::make_shared<Wolf>("Акела", 105, 12, true));
    zooManager_->addAnimal(std::make_shared<Rabbit>("Ворчун", 106, 2, 3,
                                                    true));  // Не интерактивный

    // Добавляем вещи
    zooManager_->addThing(std::make_shared<Table>("Стол на ресепшене", 201));
    zooManager_->addThing(std::make_shared<Computer>("Офисный ПК", 202));
    zooManager_->addThing(std::make_shared<Table>("Ветеринарный стол", 203));

    std::cout << "[+] Тестовые данные успешно добавлены!\n";
    std::cout << "   - 6 животных\n";
    std::cout << "   - 3 вещи\n\n";

  } catch (const std::exception& e) {
    std::cout << "[!] Ошибка при добавлении тестовых данных: " << e.what() << "\n\n";
  }
}

// ========== Утилиты для ввода ==========

int ConsoleUI::readInt(const std::string& prompt) const {
  int value;
  while (true) {
    std::cout << prompt;
    std::cin >> value;

    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "[!] Неверный ввод. Пожалуйста, введите число.\n";
    } else {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return value;
    }
  }
}

std::string ConsoleUI::readString(const std::string& prompt) const {
  std::string value;
  std::cout << prompt;
  std::getline(std::cin, value);
  return value;
}

bool ConsoleUI::readBool(const std::string& prompt) const {
  int value = readInt(prompt);
  return value != 0;
}