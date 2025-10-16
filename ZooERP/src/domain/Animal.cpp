#include "domain/Animal.h"
#include <stdexcept>
#include <sstream>

Animal::Animal(const std::string& name, int inventoryNumber, int food, bool isHealthy)
    : name_(name)
    , inventoryNumber_(inventoryNumber)
    , food_(food)
    , isHealthy_(isHealthy) {

  if (inventoryNumber <= 0) {
    throw std::invalid_argument("Inventory number must be positive");
  }

  if (food < 0) {
    throw std::invalid_argument("Food amount cannot be negative");
  }

  if (name.empty()) {
    throw std::invalid_argument("Name cannot be empty");
  }
}

void Animal::setFood(int food) {
  if (food < 0) {
    throw std::invalid_argument("Food amount cannot be negative");
  }
  food_ = food;
}

void Animal::setInventoryNumber(int number) {
  if (number <= 0) {
    throw std::invalid_argument("Inventory number must be positive");
  }
  inventoryNumber_ = number;
}

std::string Animal::getInfo() const {
  std::stringstream ss;
  ss << "Name: " << name_
     << ", Type: " << getType()
     << ", Inventory #: " << inventoryNumber_
     << ", Food: " << food_ << " kg/day"
     << ", Health: " << (isHealthy_ ? "Healthy" : "Sick");
  return ss.str();
}