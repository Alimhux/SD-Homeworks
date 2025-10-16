#ifndef TIGER_H
#define TIGER_H

#include "domain/Predator.h"

/**
 * @brief Тигр - хищное животное
 */
class Tiger : public Predator {
public:
  Tiger(const std::string& name, int inventoryNumber, int food,
        bool isHealthy = true)
      : Predator(name, inventoryNumber, food, isHealthy) {}

  std::string getType() const override { return "Tiger"; }
};

#endif // TIGER_H