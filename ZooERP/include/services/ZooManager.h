#ifndef ZOOMANAGER_H
#define ZOOMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include "interfaces/IVeterinaryClinic.h"
#include "interfaces/IZooRepository.h"
#include "domain/Animal.h"
#include "domain/Thing.h"
#include "interfaces/IInteractive.h"

/**
 * @brief Менеджер зоопарка - главная бизнес-логика
 * 
 * Применение SOLID:
 * - SRP: отвечает за управление зоопарком (добавление животных, отчёты)
 * - OCP: можно расширить функционал без изменения кода
 * - DIP: зависит от абстракций (IVeterinaryClinic, IZooRepository)
 * 
 * Внедрение зависимостей через конструктор (Constructor Injection)
 */
class ZooManager {
private:
    std::shared_ptr<IVeterinaryClinic> veterinaryClinic_;
    std::shared_ptr<IZooRepository> repository_;

public:
    /**
     * @brief Конструктор с внедрением зависимостей
     * @param clinic Ветеринарная клиника (для проверки здоровья)
     * @param repository Репозиторий (для хранения данных)
     */
    ZooManager(
        std::shared_ptr<IVeterinaryClinic> clinic,
        std::shared_ptr<IZooRepository> repository
    );
    
    virtual ~ZooManager() = default;
    
    /**
     * @brief Добавить животное в зоопарк
     * Проверяет здоровье через VeterinaryClinic
     * @param animal Животное для добавления
     * @return true если животное добавлено, false если отклонено по здоровью
     */
    bool addAnimal(std::shared_ptr<Animal> animal);
    
    /**
     * @brief Добавить вещь в зоопарк
     */
    void addThing(std::shared_ptr<Thing> thing);
    
    /**
     * @brief Рассчитать общее количество еды для всех животных
     * @return Килограммы еды в день
     */
    int calculateTotalFood() const;
    
    /**
     * @brief Получить животных для контактного зоопарка
     * @return Список интерактивных животных (доброта >= 5)
     */
    std::vector<std::shared_ptr<Animal>> getInteractiveAnimals() const;
    
    /**
     * @brief Получить всех животных
     */
    std::vector<std::shared_ptr<Animal>> getAllAnimals() const;
    
    /**
     * @brief Получить все вещи
     */
    std::vector<std::shared_ptr<Thing>> getAllThings() const;
    
    /**
     * @brief Получить общее количество животных
     */
    size_t getAnimalCount() const;
    
    /**
     * @brief Получить общее количество вещей
     */
    size_t getThingCount() const;
};

#endif // ZOOMANAGER_H