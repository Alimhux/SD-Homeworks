#include "services/ZooManager.h"
#include <stdexcept>
#include <iostream>

ZooManager::ZooManager(
    std::shared_ptr<IVeterinaryClinic> clinic,
    std::shared_ptr<IZooRepository> repository)
    : veterinaryClinic_(clinic)
    , repository_(repository) {
    
    if (!clinic) {
        throw std::invalid_argument("VeterinaryClinic cannot be null");
    }
    
    if (!repository) {
        throw std::invalid_argument("Repository cannot be null");
    }
}

bool ZooManager::addAnimal(std::shared_ptr<Animal> animal) {
    if (!animal) {
        throw std::invalid_argument("Cannot add null animal");
    }
    
    std::cout << "\n[ZooManager] Processing new animal: " << animal->getName() 
              << " (" << animal->getType() << ")" << std::endl;
    
    // Проверка здоровья через ветеринарную клинику
    bool healthCheckPassed = veterinaryClinic_->checkHealth(*animal);
    
    if (healthCheckPassed) {
        repository_->addAnimal(animal);
        return true;
    } else {
        return false;
    }
}

void ZooManager::addThing(std::shared_ptr<Thing> thing) {
    if (!thing) {
        throw std::invalid_argument("Cannot add null thing");
    }
    
    repository_->addThing(thing);
    std::cout << "[ZooManager] Thing added: " << thing->getName() << std::endl;
}

int ZooManager::calculateTotalFood() const {
    int totalFood = 0;
    
    for (const auto& animal : repository_->getAllAnimals()) {
        totalFood += animal->getFood();
    }
    
    return totalFood;
}

std::vector<std::shared_ptr<Animal>> ZooManager::getInteractiveAnimals() const {
    std::vector<std::shared_ptr<Animal>> interactiveAnimals;
    
    for (const auto& animal : repository_->getAllAnimals()) {
        // Проверяем, реализует ли животное IInteractive
        // Используем dynamic_cast для безопасного приведения типов
        auto interactive = std::dynamic_pointer_cast<IInteractive>(animal);
        
        if (interactive && interactive->isInteractive()) {
            interactiveAnimals.push_back(animal);
        }
    }
    
    return interactiveAnimals;
}

std::vector<std::shared_ptr<Animal>> ZooManager::getAllAnimals() const {
    return repository_->getAllAnimals();
}

std::vector<std::shared_ptr<Thing>> ZooManager::getAllThings() const {
    return repository_->getAllThings();
}

size_t ZooManager::getAnimalCount() const {
    return repository_->getAnimalCount();
}

size_t ZooManager::getThingCount() const {
    return repository_->getThingCount();
}