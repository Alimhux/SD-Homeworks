#include "domain/Thing.h"
#include <stdexcept>
#include <sstream>

Thing::Thing(const std::string& name, int inventoryNumber)
    : name_(name)
    , inventoryNumber_(inventoryNumber) {

  if (inventoryNumber <= 0) {
    throw std::invalid_argument("Inventory number must be positive");
  }

  if (name.empty()) {
    throw std::invalid_argument("Name cannot be empty");
  }
}

void Thing::setInventoryNumber(int number) {
  if (number <= 0) {
    throw std::invalid_argument("Inventory number must be positive");
  }
  inventoryNumber_ = number;
}

std::string Thing::getInfo() const {
  std::stringstream ss;
  ss << "Item: " << name_
     << ", Type: " << getType()
     << ", Inventory #: " << inventoryNumber_;
  return ss.str();
}



