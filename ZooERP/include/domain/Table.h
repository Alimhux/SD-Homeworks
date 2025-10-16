#ifndef TABLE_H
#define TABLE_H

#include "domain/Thing.h"

/**
 * @brief Стол - предмет мебели
 */
class Table : public Thing {
 public:
  Table(const std::string& name, int inventoryNumber)
      : Thing(name, inventoryNumber) {}

  std::string getType() const override { return "Table"; }
};

#endif  // TABLE_H