#include <gtest/gtest.h>
#include "services/ZooRepository.h"
#include "domain/Rabbit.h"
#include "domain/Tiger.h"
#include "domain/Table.h"

class ZooRepositoryTest : public ::testing::Test {
protected:
    ZooRepository repository;
    
    void SetUp() override {
        repository.clear();
    }
};

// ========== Тесты добавления животных ==========

TEST_F(ZooRepositoryTest, AddAnimal_ValidAnimal_IncreasesCount) {
    // Arrange
    auto rabbit = std::make_shared<Rabbit>("Bugs", 1, 2, 5);
    
    // Act
    repository.addAnimal(rabbit);
    
    // Assert
    EXPECT_EQ(repository.getAnimalCount(), 1);
}

TEST_F(ZooRepositoryTest, AddAnimal_MultipleAnimals_StoresAll) {
    // Arrange
    auto rabbit = std::make_shared<Rabbit>("Bugs", 1, 2, 5);
    auto tiger = std::make_shared<Tiger>("Tony", 2, 15);
    
    // Act
    repository.addAnimal(rabbit);
    repository.addAnimal(tiger);
    
    // Assert
    EXPECT_EQ(repository.getAnimalCount(), 2);
    
    auto animals = repository.getAllAnimals();
    EXPECT_EQ(animals.size(), 2);
    EXPECT_EQ(animals[0]->getName(), "Bugs");
    EXPECT_EQ(animals[1]->getName(), "Tony");
}

TEST_F(ZooRepositoryTest, AddAnimal_NullPointer_ThrowsException) {
    // Arrange, Act & Assert
    EXPECT_THROW(repository.addAnimal(nullptr), std::invalid_argument);
}

// ========== Тесты добавления вещей ==========

TEST_F(ZooRepositoryTest, AddThing_ValidThing_IncreasesCount) {
    // Arrange
    auto table = std::make_shared<Table>("Office Desk", 100);
    
    // Act
    repository.addThing(table);
    
    // Assert
    EXPECT_EQ(repository.getThingCount(), 1);
}

TEST_F(ZooRepositoryTest, AddThing_NullPointer_ThrowsException) {
    // Arrange, Act & Assert
    EXPECT_THROW(repository.addThing(nullptr), std::invalid_argument);
}

// ========== Тесты получения данных ==========

TEST_F(ZooRepositoryTest, GetAllAnimals_EmptyRepository_ReturnsEmptyVector) {
    // Arrange & Act
    auto animals = repository.getAllAnimals();
    
    // Assert
    EXPECT_TRUE(animals.empty());
}

TEST_F(ZooRepositoryTest, GetAllAnimals_WithAnimals_ReturnsAll) {
    // Arrange
    repository.addAnimal(std::make_shared<Rabbit>("Bunny1", 1, 2, 5));
    repository.addAnimal(std::make_shared<Rabbit>("Bunny2", 2, 2, 6));
    
    // Act
    auto animals = repository.getAllAnimals();
    
    // Assert
    EXPECT_EQ(animals.size(), 2);
}

// ========== Тесты очистки ==========

TEST_F(ZooRepositoryTest, Clear_RemovesAllData) {
    // Arrange
    repository.addAnimal(std::make_shared<Rabbit>("Bugs", 1, 2, 5));
    repository.addThing(std::make_shared<Table>("Desk", 100));
    
    // Act
    repository.clear();
    
    // Assert
    EXPECT_EQ(repository.getAnimalCount(), 0);
    EXPECT_EQ(repository.getThingCount(), 0);
}