#pragma once

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "application/facades/account_facade.h"
#include "application/facades/analytics_facade.h"
#include "application/facades/operation_facade.h"
#include "infrastructure/di/di_container.h"

namespace financial::presentation {

using namespace financial::application;

// Console UI class following Controller pattern
class ConsoleUI {
 private:
  std::shared_ptr<AccountFacade> accountFacade_;
  std::shared_ptr<OperationFacade> operationFacade_;
  std::shared_ptr<CategoryFacade> categoryFacade_;
  std::shared_ptr<AnalyticsFacade> analyticsFacade_;

  bool running_ = true;

 public:
  ConsoleUI() {
    accountFacade_ = std::make_shared<AccountFacade>();
    operationFacade_ = std::make_shared<OperationFacade>();
    categoryFacade_ = std::make_shared<CategoryFacade>();
    analyticsFacade_ = std::make_shared<AnalyticsFacade>();
  }

  void run() {
    displayWelcome();

    while (running_) {
      displayMainMenu();
      int choice = getUserChoice(1, 9);

      switch (choice) {
        case 1:
          accountMenu();
          break;
        case 2:
          categoryMenu();
          break;
        case 3:
          operationMenu();
          break;
        // case 0: analyticsMenu(); break;
        // case 0: importExportMenu(); break;
        // case 0: runQuickDemo(); break;
        case 4:
          demonstratePatterns();
          break;
        case 5:
          displayHelp();
          break;
        case 6:
          running_ = false;
          break;
      }
    }

    displayGoodbye();
  }

 private:
  void displayWelcome() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║         ВШЭ-БАНК: СИСТЕМА УЧЕТА ФИНАНСОВ           ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n";
  }

  void displayMainMenu() {
    std::cout << "\n=== ГЛАВНОЕ МЕНЮ ===\n";
    std::cout << "1. Управление счетами\n";
    std::cout << "2. Управление категориями\n";
    std::cout << "3. Управление операциями\n";
    // std::cout << "4. Аналитика и отчеты\n";
    // std::cout << "5. Импорт/Экспорт данных\n";
    // std::cout << "4. Быстрая демонстрация\n";
    std::cout << "4. Демонстрация паттернов\n";
    std::cout << "5. Справка\n";
    std::cout << "6. Выход\n";
  }

  void accountMenu() {
    bool inMenu = true;
    while (inMenu) {
      std::cout << "\n=== УПРАВЛЕНИЕ СЧЕТАМИ ===\n";
      std::cout << "1. Создать счет\n";
      std::cout << "2. Список счетов\n";
      std::cout << "3. Пополнить счет\n";
      std::cout << "4. Снять со счета\n";
      std::cout << "5. Перевод между счетами\n";
      std::cout << "6. Удалить счет\n";
      std::cout << "7. Назад\n";

      int choice = getUserChoice(1, 7);

      switch (choice) {
        case 1:
          createAccount();
          break;
        case 2:
          listAccounts();
          break;
        case 3:
          depositToAccount();
          break;
        case 4:
          withdrawFromAccount();
          break;
        case 5:
          transferBetweenAccounts();
          break;
        case 6:
          deleteAccount();
          break;
        case 7:
          inMenu = false;
          break;
      }
    }
  }

  void categoryMenu() {
    bool inMenu = true;
    while (inMenu) {
      std::cout << "\n=== УПРАВЛЕНИЕ КАТЕГОРИЯМИ ===\n";
      std::cout << "1. Создать категорию\n";
      std::cout << "2. Список категорий\n";
      std::cout << "3. Создать стандартные категории\n";
      std::cout << "4. Удалить категорию\n";
      std::cout << "5. Назад\n";

      int choice = getUserChoice(1, 5);

      switch (choice) {
        case 1:
          createCategory();
          break;
        case 2:
          listCategories();
          break;
        case 3:
          createDefaultCategories();
          break;
        case 4:
          deleteCategory();
          break;
        case 5:
          inMenu = false;
          break;
      }
    }
  }

  void operationMenu() {
    bool inMenu = true;
    while (inMenu) {
      std::cout << "\n=== УПРАВЛЕНИЕ ОПЕРАЦИЯМИ ===\n";
      std::cout << "1. Добавить доход\n";
      std::cout << "2. Добавить расход\n";
      std::cout << "3. Список операций\n";
      std::cout << "4. Операции за сегодня\n";
      std::cout << "5. Операции за месяц\n";
      std::cout << "6. Удалить операцию\n";
      std::cout << "7. Назад\n";

      int choice = getUserChoice(1, 7);

      switch (choice) {
        case 1:
          addIncome();
          break;
        case 2:
          addExpense();
          break;
        case 3:
          listOperations();
          break;
        case 4:
          showTodayOperations();
          break;
        case 5:
          showMonthOperations();
          break;
        case 6:
          deleteOperation();
          break;
        case 7:
          inMenu = false;
          break;
      }
    }
  }

  void analyticsMenu() {
    bool inMenu = true;
    while (inMenu) {
      std::cout << "\n=== АНАЛИТИКА И ОТЧЕТЫ ===\n";
      std::cout << "1. Отчет за месяц\n";
      std::cout << "2. Отчет за год\n";
      std::cout << "3. Топ категорий доходов\n";
      std::cout << "4. Топ категорий расходов\n";
      std::cout << "5. Проверить балансы\n";
      std::cout << "6. Статистика производительности\n";
      std::cout << "7. Назад\n";

      int choice = getUserChoice(1, 7);

      switch (choice) {
        case 1:
          showMonthlyReport();
          break;
        case 2:
          showYearlyReport();
          break;
        case 3:
          showTopIncomeCategories();
          break;
        case 4:
          showTopExpenseCategories();
          break;
        case 5:
          checkBalances();
          break;
        case 6:
          showPerformanceStats();
          break;
        case 7:
          inMenu = false;
          break;
      }
    }
  }

  void importExportMenu() {
    bool inMenu = true;
    while (inMenu) {
      std::cout << "\n=== ИМПОРТ/ЭКСПОРТ ДАННЫХ ===\n";
      std::cout << "1. Экспорт в CSV\n";
      std::cout << "2. Экспорт в JSON\n";
      std::cout << "3. Импорт из CSV\n";
      std::cout << "4. Импорт из JSON\n";
      std::cout << "5. Назад\n";

      int choice = getUserChoice(1, 5);

      switch (choice) {
        case 1:
          exportToCSV();
          break;
        case 2:
          exportToJSON();
          break;
        case 3:
          importFromCSV();
          break;
        case 4:
          importFromJSON();
          break;
        case 5:
          inMenu = false;
          break;
      }
    }
  }

  // Implementation of menu actions
  void createAccount() {
    std::cout << "\n--- Создание счета ---\n";
    std::string name = getUserInput("Введите название счета: ");
    double balance = getUserDouble("Начальный баланс (0 для пустого): ");

    try {
      auto account = accountFacade_->createAccount(name, balance);
      std::cout << "✓ Счет создан успешно! ID: " << account->getId() << "\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void listAccounts() {
    auto accounts = accountFacade_->getAllAccounts();
    if (accounts.empty()) {
      std::cout << "Нет созданных счетов.\n";
      return;
    }
    std::cout << "\n--- Список счетов ---\n";

    const int idWidth = 22;
    const int nameWidth = 12;
    const int balanceWidth =
        15;
    const int statusWidth = 8;

    // Заголовки
    std::cout << std::left << std::setw(idWidth) << "ID" << std::setw(nameWidth)
              << " Название    "
              << std::right
              << std::setw(balanceWidth) << "     Баланс" << "        "
              << std::left
              << std::setw(statusWidth) << "  Статус"
              << "\n";

    // Разделитель
    std::cout << std::string(idWidth + nameWidth + balanceWidth + statusWidth,
                             '-')
              << "\n";

    for (const auto& account : accounts) {
      std::stringstream balanceStream;
      balanceStream << std::fixed << std::setprecision(2)
                    << account->getBalance().getAmount() << " "
                    << account->getCurrency();
      std::string formattedBalance = balanceStream.str();

      std::cout << std::left << std::setw(idWidth) << account->getId() << "    "
                << std::setw(nameWidth) << account->getName()
                << std::setw(balanceWidth) << formattedBalance
                << " "  // Небольшой отступ перед статусом
                << std::setw(statusWidth - 1)  // -1 из-за отступа " "
                << (account->getIsActive() ? "Активен" : "Неактивен") << "\n";
    }
  }

  void createCategory() {
    std::cout << "\n--- Создание категории ---\n";
    std::cout << "Тип категории:\n";
    std::cout << "1. Доход\n";
    std::cout << "2. Расход\n";

    int typeChoice = getUserChoice(1, 2);
    CategoryType type =
        (typeChoice == 1) ? CategoryType::INCOME : CategoryType::EXPENSE;

    std::string name = getUserInput("Название категории: ");
    std::string description = getUserInput("Описание (опционально): ");

    try {
      auto category = categoryFacade_->createCategory(type, name, description);
      std::cout << "✓ Категория создана успешно! ID: " << category->getId()
                << "\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void createDefaultCategories() {
    try {
      categoryFacade_->createDefaultCategories();
      std::cout << "✓ Стандартные категории созданы успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void addIncome() {
    listAccounts();
    std::string accountId = getUserInput("\nВведите ID счета: ");

    listCategories(CategoryType::INCOME);
    std::string categoryId = getUserInput("\nВведите ID категории: ");

    double amount = getUserDouble("Сумма дохода: ");
    std::string description = getUserInput("Описание: ");

    try {
      auto operation = operationFacade_->addIncome(accountId, amount,
                                                   categoryId, description);
      std::cout << "✓ Доход добавлен успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void addExpense() {
    listAccounts();
    std::string accountId = getUserInput("\nВведите ID счета: ");

    listCategories(CategoryType::EXPENSE);
    std::string categoryId = getUserInput("\nВведите ID категории: ");

    double amount = getUserDouble("Сумма расхода: ");
    std::string description = getUserInput("Описание: ");

    try {
      auto operation = operationFacade_->addExpense(accountId, amount,
                                                    categoryId, description);
      std::cout << "✓ Расход добавлен успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void showMonthlyReport() {
    auto report = analyticsFacade_->generateMonthlyReport();
    std::cout << "\n" << report << "\n";
  }

  void showPerformanceStats() {
    auto& stats = PerformanceStatistics::getInstance();
    std::cout << "\n" << stats.generateReport() << "\n";
  }

  void demonstratePatterns() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ ПАТТЕРНОВ ===\n\n";

    std::cout << "Реализованные паттерны проектирования:\n";
    std::cout << "---------------------------------------\n\n";

    std::cout << "1. FACTORY METHOD - EntityFactory\n";
    std::cout << "   Централизованное создание сущностей с валидацией\n\n";

    std::cout << "2. SINGLETON - DIContainer\n";
    std::cout << "   Управление зависимостями и конфигурацией сервисов\n\n";

    std::cout << "3. PROXY - CachingRepositoryProxy\n";
    std::cout << "   Кэширование доступа к репозиториям\n\n";

    std::cout << "4. COMMAND - операции как объекты\n";
    std::cout << "   Поддержка отмены/повтора операций\n\n";

    std::cout << "5. DECORATOR - измерение производительности\n";
    std::cout << "   Логирование и профилирование команд\n\n";

    std::cout << "6. FACADE - упрощенный интерфейс\n";
    std::cout << "   AccountFacade, OperationFacade, AnalyticsFacade\n\n";

    std::cout << "7. TEMPLATE METHOD - импорт данных\n";
    std::cout << "   Общий алгоритм с различными форматами\n\n";

    std::cout << "8. VISITOR - экспорт данных\n";
    std::cout << "   Обход структуры для разных форматов\n\n";

    std::cout << "9. BUILDER - создание отчетов\n";
    std::cout << "   Пошаговое построение сложных отчетов\n\n";

    std::cout << "Нажмите Enter для продолжения...";
    std::cin.ignore();
    std::cin.get();
  }

  void runQuickDemo() {
    std::cout << "\n=== БЫСТРАЯ ДЕМОНСТРАЦИЯ ===\n";

    try {
      // Создаем стандартные категории
      std::cout << "\n1. Создание стандартных категорий...\n";
      categoryFacade_->createDefaultCategories();
      std::cout << "   ✓ Категории созданы\n";

      // Создаем счета
      std::cout << "\n2. Создание счетов...\n";
      auto mainAccount = accountFacade_->createAccount("Основной счет", 100000);
      auto savingsAccount = accountFacade_->createSavingsAccount("Накопления");
      std::cout << "   ✓ Счета созданы\n";

      // Добавляем операции
      std::cout << "\n3. Добавление операций...\n";
      auto salaryCategory = categoryFacade_->getCategoryByName("Salary");
      auto foodCategory = categoryFacade_->getCategoryByName("Food");

      if (salaryCategory && foodCategory) {
        operationFacade_->addIncome(mainAccount->getId(), 50000,
                                    salaryCategory->getId(), "Зарплата");
        operationFacade_->addExpense(mainAccount->getId(), 5000,
                                     foodCategory->getId(), "Продукты");
        std::cout << "   ✓ Операции добавлены\n";
      }

      // Перевод между счетами
      std::cout << "\n4. Перевод между счетами...\n";
      accountFacade_->transfer(mainAccount->getId(), savingsAccount->getId(),
                               20000);
      std::cout << "   ✓ Перевод выполнен\n";

      // Показываем отчет
      std::cout << "\n5. Генерация отчета...\n";
      auto report = analyticsFacade_->generateMonthlyReport();
      std::cout << report << "\n";

      std::cout << "\nДемонстрация завершена!\n";

    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }

    std::cout << "\nНажмите Enter для продолжения...";
    std::cin.ignore();
    std::cin.get();
  }

  // Helper methods
  int getUserChoice(int min, int max) {
    int choice;
    while (true) {
      std::cout << "Выберите опцию (" << min << "-" << max << "): ";
      std::cin >> choice;

      if (std::cin.fail() || choice < min || choice > max) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Неверный выбор. Попробуйте снова.\n";
      } else {
        std::cin.ignore();
        return choice;
      }
    }
  }

  std::string getUserInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
  }

  double getUserDouble(const std::string& prompt) {
    double value;
    while (true) {
      std::cout << prompt;
      std::cin >> value;

      if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Неверный формат числа. Попробуйте снова.\n";
      } else {
        std::cin.ignore();
        return value;
      }
    }
  }

  // Additional helper methods
  void listCategories(CategoryType type) {
    auto categories = (type == CategoryType::INCOME)
                          ? categoryFacade_->getIncomeCategories()
                          : categoryFacade_->getExpenseCategories();

    std::cout << "\n--- Категории "
              << (type == CategoryType::INCOME ? "доходов" : "расходов")
              << " ---\n";
    for (const auto& cat : categories) {
      std::cout << cat->getId() << " - " << cat->getName() << "\n";
    }
  }

  void listCategories() {
    // std::cout << "\n--- Категории доходов ---\n";
    listCategories(CategoryType::INCOME);
    // std::cout << "\n--- Категории расходов ---\n";
    listCategories(CategoryType::EXPENSE);
  }

  void depositToAccount() {
    listAccounts();
    std::string accountId = getUserInput("\nВведите ID счета: ");
    double amount = getUserDouble("Сумма пополнения: ");

    try {
      accountFacade_->deposit(accountId, amount);
      std::cout << "✓ Счет пополнен успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void withdrawFromAccount() {
    listAccounts();
    std::string accountId = getUserInput("\nВведите ID счета: ");
    double amount = getUserDouble("Сумма снятия: ");

    try {
      accountFacade_->withdraw(accountId, amount);
      std::cout << "✓ Средства сняты успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void transferBetweenAccounts() {
    listAccounts();
    std::string fromId = getUserInput("\nID счета отправителя: ");
    std::string toId = getUserInput("ID счета получателя: ");
    double amount = getUserDouble("Сумма перевода: ");

    try {
      accountFacade_->transfer(fromId, toId, amount);
      std::cout << "✓ Перевод выполнен успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void deleteAccount() {
    listAccounts();
    std::string accountId = getUserInput("\nВведите ID счета для удаления: ");

    try {
      accountFacade_->deleteAccount(accountId);
      std::cout << "✓ Счет удален успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void deleteCategory() {
    listCategories();
    std::string categoryId =
        getUserInput("\nВведите ID категории для удаления: ");

    try {
      categoryFacade_->deleteCategory(categoryId);
      std::cout << "✓ Категория удалена успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void listOperations() {
    listAccounts();
    std::string accountId =
        getUserInput("\nВведите ID счета (пусто для всех): ");

    // Implementation simplified
    std::cout << "Список операций...\n";
  }

  void showTodayOperations() {
    auto operations = operationFacade_->getTodayOperations();
    std::cout << "\nОпераций за сегодня: " << operations.size() << "\n";
  }

  void showMonthOperations() {
    auto operations = operationFacade_->getMonthOperations();
    std::cout << "\nОпераций за месяц: " << operations.size() << "\n";
  }

  void deleteOperation() {
    std::string operationId =
        getUserInput("Введите ID операции для удаления: ");

    try {
      operationFacade_->deleteOperation(operationId);
      std::cout << "✓ Операция удалена успешно!\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void showYearlyReport() {
    auto report = analyticsFacade_->generateYearlyReport();
    std::cout << "\n" << report << "\n";
  }

  void showTopIncomeCategories() {
    auto categories = analyticsFacade_->getTopIncomeCategories(5);
    std::cout << "\n--- Топ категорий доходов ---\n";
    for (const auto& cat : categories) {
      std::cout << cat.categoryName << ": " << cat.totalAmount.getAmount()
                << "\n";
    }
  }

  void showTopExpenseCategories() {
    auto categories = analyticsFacade_->getTopExpenseCategories(5);
    std::cout << "\n--- Топ категорий расходов ---\n";
    for (const auto& cat : categories) {
      std::cout << cat.categoryName << ": " << cat.totalAmount.getAmount()
                << "\n";
    }
  }

  void checkBalances() {
    auto balances = analyticsFacade_->checkAllBalances();
    std::cout << "\n--- Проверка балансов ---\n";
    for (const auto& balance : balances) {
      std::cout << balance.accountName << ": ";
      if (balance.hasDiscrepancy) {
        std::cout << "✗ Несоответствие! ";
        std::cout << "Текущий: " << balance.balance.getAmount();
        std::cout << ", Расчетный: " << balance.calculatedBalance.getAmount()
                  << "\n";
      } else {
        std::cout << "✓ OK (" << balance.balance.getAmount() << ")\n";
      }
    }
  }

  void exportToCSV() {
    std::string filename = getUserInput("Имя файла (с расширением .csv): ");
    try {
      analyticsFacade_->exportToCSV(filename);
      std::cout << "✓ Данные экспортированы в " << filename << "\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void exportToJSON() {
    std::string filename = getUserInput("Имя файла (с расширением .json): ");
    try {
      analyticsFacade_->exportToJSON(filename);
      std::cout << "✓ Данные экспортированы в " << filename << "\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void importFromCSV() {
    std::string filename = getUserInput("Имя файла CSV: ");
    try {
      analyticsFacade_->importFromCSV(filename);
      std::cout << "✓ Данные импортированы из " << filename << "\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void importFromJSON() {
    std::string filename = getUserInput("Имя файла JSON: ");
    try {
      analyticsFacade_->importFromJSON(filename);
      std::cout << "✓ Данные импортированы из " << filename << "\n";
    } catch (const std::exception& e) {
      std::cout << "✗ Ошибка: " << e.what() << "\n";
    }
  }

  void displayHelp() {
    std::cout << "\n=== СПРАВКА ===\n";
    std::cout << "Система учета финансов ВШЭ-Банк\n";
    std::cout << "--------------------------------\n";
    std::cout << "Основные возможности:\n";
    std::cout << "• Управление счетами и категориями\n";
    std::cout << "• Учет доходов и расходов\n";
    // std::cout << "• Аналитика и отчетность\n";
    // std::cout << "• Импорт/экспорт данных\n";
    std::cout << "• Поддержка отмены операций\n";
    std::cout << "\nИспользуйте числовые команды для навигации.\n";
  }

  void displayGoodbye() {
    std::cout << "\n╔════════════════════════════════════════════════════╗\n";
    std::cout << "║         Спасибо за использование системы!         ║\n";
    std::cout << "║                    До свидания!                   ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
  }
};

}  // namespace financial::presentation