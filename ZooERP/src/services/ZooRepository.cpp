#include "services/ZooRepository.h"
#include <stdexcept>

void ZooRepository::addAnimal(std::shared_ptr<Animal> animal) {
  if (!animal) {
    throw std::invalid_argument("Cannot add null animal");
  }
  animals_.push_back(animal);
}

void ZooRepository::addThing(std::shared_ptr<Thing> thing) {
  if (!thing) {
    throw std::invalid_argument("Cannot add null thing");
  }
  things_.push_back(thing);
}

std::vector<std::shared_ptr<Animal>> ZooRepository::getAllAnimals() const {
  return animals_;
}

std::vector<std::shared_ptr<Thing>> ZooRepository::getAllThings() const {
  return things_;
}

size_t ZooRepository::getAnimalCount() const {
  return animals_.size();
}

size_t ZooRepository::getThingCount() const {
  return things_.size();
}

void ZooRepository::clear() {
  animals_.clear();
  things_.clear();
}