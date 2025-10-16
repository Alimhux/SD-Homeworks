#ifndef MONKEY_H
#define MONKEY_H

#include "domain/Herbivore.h"
/*
 * @brief Обезьяна - травоядное животное (может быть всеядной, но для простоты -
 * травоядное)
 */
class Monkey : public Herbivore {
 public:
  Monkey(const std::string& name, int inventoryNumber, int food, int kindness,
         bool isHealthy = true)
      : Herbivore(name, inventoryNumber, food, kindness, isHealthy) {}
  std::string getType() const override { return "Monkey"; }
};

#endif  // MONKEY_H
