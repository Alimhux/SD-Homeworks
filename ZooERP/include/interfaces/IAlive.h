#ifndef IALIVE_H
#define IALIVE_H

/**
 * Интерфейс для живых существ
 *
 * Применение ISP: отвечает только за питание
 * Применение DIP: абстракция для работы с потреблением пищи
 */

class IAlive {
public:
  virtual ~IAlive() = default;

  [[nodiscard]] virtual int getFood() const = 0;

  virtual void setFood(int food) = 0;
};

#endif //IALIVE_H
