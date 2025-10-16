#ifndef IHEALTHCHECKABLE_H
#define IHEALTHCHECKABLE_H


/**
 * Интерфейс для объектов, которые можно проверить на здоровье
 *
 * Применение ISP: отдельный интерфейс для медицинских проверок
 * Применение DIP: VeterinaryClinic будет зависеть от этой абстракции
 */

class IHealthCheckable {
public:
  virtual ~IHealthCheckable() = default;

  [[nodiscard]] virtual bool isHealthy() const = 0;
};


#endif //IHEALTHCHECKABLE_H
