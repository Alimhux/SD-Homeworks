#ifndef COMPUTER_H
#define COMPUTER_H

#include "domain/Thing.h"

/**
 * @brief Компьютер - офисная техника
 */
class Computer : public Thing {
public:
  Computer(const std::string& name, int inventoryNumber)
      : Thing(name, inventoryNumber) {}

  std::string getType() const override { return "Computer"; }
};

#endif // COMPUTER_H