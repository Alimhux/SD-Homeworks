#ifndef ZOOREPOSITORY_H
#define ZOOREPOSITORY_H

#include <memory>
#include <vector>

#include "interfaces/IZooRepository.h"

/**
 * @brief Реализация репозитория зоопарка
 *
 * Применение SRP: отвечает только за хранение данных
 * Хранит коллекции животных и вещей в памяти
 */
class ZooRepository : public IZooRepository {
 private:
  std::vector<std::shared_ptr<Animal>> animals_;
  std::vector<std::shared_ptr<Thing>> things_;

 public:
  ZooRepository() = default;
  virtual ~ZooRepository() = default;

  void addAnimal(std::shared_ptr<Animal> animal) override;
  void addThing(std::shared_ptr<Thing> thing) override;

  std::vector<std::shared_ptr<Animal>> getAllAnimals() const override;
  std::vector<std::shared_ptr<Thing>> getAllThings() const override;

  size_t getAnimalCount() const override;
  size_t getThingCount() const override;

  void clear() override;
};

#endif  // ZOOREPOSITORY_H