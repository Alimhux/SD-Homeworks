#ifndef RABBIT_H
#define RABBIT_H

#include "domain/Herbivore.h"

class Rabbit : public Herbivore {
 public:
  Rabbit(const std::string& name, int inventoryNumber, int food, int kindness,
         bool isHealthy = true)
      : Herbivore(name, inventoryNumber, food, kindness, isHealthy) {}

  std::string getType() const override { return "Rabbit"; }
};

#endif  // RABBIT_H
