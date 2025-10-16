#ifndef WOLF_H
#define WOLF_H

#include "domain/Predator.h"

/**
 * @brief Волк - хищное животное
 */
class Wolf : public Predator {
public:
  Wolf(const std::string& name, int inventoryNumber, int food,
       bool isHealthy = true)
      : Predator(name, inventoryNumber, food, isHealthy) {}

  std::string getType() const override { return "Wolf"; }
};

#endif // WOLF_H