#include "services/VeterinaryClinic.h"
#include <iostream>

bool VeterinaryClinic::checkHealth(const IHealthCheckable& animal) {
  // Простая проверка здоровья
  // В реальном приложении здесь могла бы быть сложная логика

  bool healthy = animal.isHealthy();

  if (healthy) {
    std::cout << "[VeterinaryClinic] Animal passed health check ✓" << std::endl;
  } else {
    std::cout << "[VeterinaryClinic] Animal failed health check ✗" << std::endl;
  }

  return healthy;
}