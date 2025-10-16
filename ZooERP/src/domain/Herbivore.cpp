#include "domain/Herbivore.h"
#include <stdexcept>
#include <sstream>

Herbivore::Herbivore(const std::string& name, int inventoryNumber, int food,
                     int kindness, bool isHealthy)
    : Animal(name, inventoryNumber, food, isHealthy)
    , kindness_(kindness) {

  // Валидация уровня доброты
  if (kindness < 0 || kindness > 10) {
    throw std::invalid_argument("Kindness must be between 0 and 10");
  }
}

bool Herbivore::isInteractive() const {
  // Животное может быть в контактном зоопарке, если доброта >= 5
  return kindness_ >= 5;
}

void Herbivore::setKindness(int kindness) {
  if (kindness < 0 || kindness > 10) {
    throw std::invalid_argument("Kindness must be between 0 and 10");
  }
  kindness_ = kindness;
}

std::string Herbivore::getInfo() const {
  std::stringstream ss;
  ss << Animal::getInfo()  // Вызываем базовую версию
     << ", Kindness: " << kindness_ << "/10"
     << ", Interactive: " << (isInteractive() ? "Yes" : "No");
  return ss.str();
}