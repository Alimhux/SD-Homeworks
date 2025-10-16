#include <gtest/gtest.h>
#include "domain/Tiger.h"
#include "domain/Wolf.h"

// ========== Тесты Tiger ==========

TEST(TigerTest, Constructor_ValidData_Success) {
    // Arrange & Act
    Tiger tiger("Shere Khan", 10, 15, true);
    
    // Assert
    EXPECT_EQ(tiger.getName(), "Shere Khan");
    EXPECT_EQ(tiger.getInventoryNumber(), 10);
    EXPECT_EQ(tiger.getFood(), 15);
    EXPECT_EQ(tiger.getType(), "Tiger");
    EXPECT_TRUE(tiger.isHealthy());
}

TEST(TigerTest, GetType_ReturnsTiger) {
    // Arrange
    Tiger tiger("Rajah", 11, 12);
    
    // Act & Assert
    EXPECT_EQ(tiger.getType(), "Tiger");
}

TEST(TigerTest, GetInfo_ContainsCorrectInfo) {
    // Arrange
    Tiger tiger("Tony", 12, 18);
    
    // Act
    std::string info = tiger.getInfo();
    
    // Assert
    EXPECT_NE(info.find("Tony"), std::string::npos);
    EXPECT_NE(info.find("Tiger"), std::string::npos);
    EXPECT_NE(info.find("18 kg"), std::string::npos);
}

// ========== Тесты Wolf ==========

TEST(WolfTest, Constructor_ValidData_Success) {
    // Arrange & Act
    Wolf wolf("Akela", 20, 10, true);
    
    // Assert
    EXPECT_EQ(wolf.getName(), "Akela");
    EXPECT_EQ(wolf.getInventoryNumber(), 20);
    EXPECT_EQ(wolf.getFood(), 10);
    EXPECT_EQ(wolf.getType(), "Wolf");
}

TEST(WolfTest, GetType_ReturnsWolf) {
    // Arrange
    Wolf wolf("Grey", 21, 8);
    
    // Act & Assert
    EXPECT_EQ(wolf.getType(), "Wolf");
}

TEST(WolfTest, SetFood_UpdatesCorrectly) {
    // Arrange
    Wolf wolf("White Fang", 22, 10);
    
    // Act
    wolf.setFood(12);
    
    // Assert
    EXPECT_EQ(wolf.getFood(), 12);
}

// ========== Тесты полиморфизма ==========

TEST(PredatorTest, Polymorphism_AnimalPointer_WorksCorrectly) {
    // Arrange - используем указатель на базовый класс
    std::shared_ptr<Animal> animal = std::make_shared<Tiger>("Bengal", 30, 20);
    
    // Act & Assert
    EXPECT_EQ(animal->getType(), "Tiger");
    EXPECT_EQ(animal->getFood(), 20);
    EXPECT_EQ(animal->getName(), "Bengal");
}

TEST(PredatorTest, Polymorphism_MultipleAnimals_AllWork) {
    // Arrange - создаём коллекцию разных животных
    std::vector<std::shared_ptr<Animal>> animals;
    animals.push_back(std::make_shared<Tiger>("Tiger1", 1, 15));
    animals.push_back(std::make_shared<Wolf>("Wolf1", 2, 10));
    
    // Act - обращаемся полиморфно
    int totalFood = 0;
    for (const auto& animal : animals) {
        totalFood += animal->getFood();
    }
    
    // Assert
    EXPECT_EQ(totalFood, 25);
    EXPECT_EQ(animals[0]->getType(), "Tiger");
    EXPECT_EQ(animals[1]->getType(), "Wolf");
}