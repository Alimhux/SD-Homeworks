#include <iostream>
#include <exception>
#include "infrastructure/di/di_container.h"
#include "presentation/console/console_ui.h"
#include "application/decorators/command_decorators.h"

using namespace financial::infrastructure;
using namespace financial::presentation;
using namespace financial::application;

int main() {
  try {
    // Инициализация системы логирования
    LoggingDecorator::openLogFile("financial_app.log");

    // Конфигурация DI контейнера
    std::cout << "Инициализация системы...\n";
    ServiceConfigurator::configureServices(true); // с кэшированием

    // Запуск консольного интерфейса
    ConsoleUI ui{};
    ui.run();

    // Вывод статистики производительности перед выходом
    auto& stats = PerformanceStatistics::getInstance();
    std::string perfReport = stats.generateReport();
    if (!perfReport.empty()) {
      std::cout << "\n" << perfReport;
    }

    // Закрытие логов
    LoggingDecorator::closeLogFile();

  } catch (const std::exception& e) {
    std::cerr << "Критическая ошибка: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Неизвестная критическая ошибка!" << std::endl;
    return 2;
  }

  return 0;
}