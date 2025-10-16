#ifndef HERBIVORE_H
#define HERBIVORE_H

#include "domain/Animal.h"
#include "interfaces/IInteractive.h"

class Herbivore : public Animal, public IInteractive {
 protected:
  int kindness_;

 public:
  Herbivore(const std::string& name, int inventoryNumber, int food,
            int kindness, bool isHealty);

  virtual ~Herbivore() = default;

  // Реализация IInteractive
  [[nodiscard]] bool isInteractive() const override;

  // Геттеры/сеттеры для доброты
  [[nodiscard]] int getKindness() const { return kindness_; }
  void setKindness(int kindness);

  // Переопределяем getInfo для добавления информации о доброте
  [[nodiscard]] std::string getInfo() const override;

  // Реализация getType (пока абстрактный, конкретные животные переопределят)
  [[nodiscard]] std::string getType() const override = 0;
};

#endif  // HERBIVORE_H
