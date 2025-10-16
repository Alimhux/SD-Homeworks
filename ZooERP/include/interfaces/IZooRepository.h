#ifndef IZOOREPOSITORY_H
#define IZOOREPOSITORY_H

#include <memory>
#include <vector>
#include "domain/Animal.h"
#include "domain/Thing.h"

/**
 * @brief Интерфейс репозитория для хранения данных зоопарка
 *
 * Применение DIP: бизнес-логика зависит от абстракции хранилища
 */
class IZooRepository {
public:
  virtual ~IZooRepository() = default;

  /**
   * @brief Добавить животное в зоопарк
   */
  virtual void addAnimal(std::shared_ptr<Animal> animal) = 0;

  /**
   * @brief Добавить вещь в зоопарк
   */
  virtual void addThing(std::shared_ptr<Thing> thing) = 0;

  /**
   * @brief Получить всех животных
   */
  virtual std::vector<std::shared_ptr<Animal>> getAllAnimals() const = 0;

  /**
   * @brief Получить все вещи
   */
  virtual std::vector<std::shared_ptr<Thing>> getAllThings() const = 0;

  /**
   * @brief Получить количество животных
   */
  virtual size_t getAnimalCount() const = 0;

  /**
   * @brief Получить количество вещей
   */
  virtual size_t getThingCount() const = 0;

  /**
   * @brief Очистить все данные (для тестов)
   */
  virtual void clear() = 0;
};

#endif // IZOOREPOSITORY_H