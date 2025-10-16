#ifndef ANIMAL_H
#define ANIMAL_H

#include <string>
#include <memory>
#include "interfaces/IAlive.h"
#include "interfaces/IInventory.h"
#include "interfaces/IHealthCheckable.h"

/**
 * @brief Абстрактный базовый класс для всех животных
 *
 * Применение SOLID:
 * - SRP: отвечает за хранение базовых данных животного
 * - OCP: открыт для расширения (Herbivore, Predator), закрыт для модификации
 * - LSP: все потомки могут использоваться вместо Animal
 * - DIP: зависит от интерфейсов, а не конкретных реализаций
 */
class Animal : public IAlive, public IInventory, public IHealthCheckable {
protected:
    std::string name_;
    int inventoryNumber_;
    int food_;
    bool isHealthy_;

public:
    /**
     * @brief Конструктор животного
     * @param name Имя животного
     * @param inventoryNumber Уникальный номер (> 0)
     * @param food Количество еды в кг (>= 0)
     * @param isHealthy Здоровье (по умолчанию true)
     */
    Animal(const std::string& name, int inventoryNumber, int food, bool isHealthy = true);

    virtual ~Animal() = default;

    // Реализация IAlive
    int getFood() const override { return food_; }
    void setFood(int food) override;

    // Реализация IInventory
    int getInventoryNumber() const override { return inventoryNumber_; }
    void setInventoryNumber(int number) override;

    // Реализация IHealthCheckable
    bool isHealthy() const override { return isHealthy_; }

    // Дополнительные методы
    std::string getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    void setHealthy(bool healthy) { isHealthy_ = healthy; }

    /**
     * @brief Получить информацию о животном (для вывода)
     * Виртуальный метод - потомки могут добавлять свою информацию
     */
    virtual std::string getInfo() const;

    /**
     * @brief Получить тип животного (для полиморфизма)
     * Чисто виртуальный - каждый потомок обязан реализовать
     */
    virtual std::string getType() const = 0;
};

#endif // ANIMAL_H