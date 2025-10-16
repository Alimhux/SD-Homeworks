#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/ZooManager.h"
#include "services/VeterinaryClinic.h"
#include "services/ZooRepository.h"
#include "domain/Rabbit.h"
#include "domain/Tiger.h"
#include "domain/Monkey.h"
#include "domain/Table.h"

// Моки для изолированного тестирования
class MockVeterinaryClinic : public IVeterinaryClinic {
public:
    MOCK_METHOD(bool, checkHealth, (const IHealthCheckable&), (override));
};

class MockZooRepository : public IZooRepository {
public:
    MOCK_METHOD(void, addAnimal, (std::shared_ptr<Animal>), (override));
    MOCK_METHOD(void, addThing, (std::shared_ptr<Thing>), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Animal>>, getAllAnimals, (), (const, override));
    MOCK_METHOD(std::vector<std::shared_ptr<Thing>>, getAllThings, (), (const, override));
    MOCK_METHOD(size_t, getAnimalCount, (), (const, override));
    MOCK_METHOD(size_t, getThingCount, (), (const, override));
    MOCK_METHOD(void, clear, (), (override));
};

// ========== Тесты конструктора ==========

TEST(ZooManagerTest, Constructor_NullClinic_ThrowsException) {
    // Arrange
    auto repo = std::make_shared<ZooRepository>();
    
    // Act & Assert
    EXPECT_THROW(
        ZooManager(nullptr, repo),
        std::invalid_argument
    );
}

TEST(ZooManagerTest, Constructor_NullRepository_ThrowsException) {
    // Arrange
    auto clinic = std::make_shared<VeterinaryClinic>();
    
    // Act & Assert
    EXPECT_THROW(
        ZooManager(clinic, nullptr),
        std::invalid_argument
    );
}

// ========== Тесты добавления животных с моками ==========

//TEST(ZooManagerTest, AddAnimal_HealthyAnimal_ReturnsTrue) {
//    // Arrange
//    auto mockClinic = std::make_shared<MockVeterinaryClinic>();
//    auto mockRepo = std::make_shared<MockZooRepository>();
//
//    auto rabbit = std::make_shared<Rabbit>("Bugs", 1, 2, 5);
//
//    // Настройка моков
//    EXPECT_CALL(*mockClinic, checkHealth(testing::_))
//        .WillOnce(testing::Return(true));
//
//    EXPECT_CALL(*mockRepo, addAnimal(rabbit))
//        .Times(1);
//
//    ZooManager manager(mockClinic, mockRepo);
//
//    // Act
//    bool result = manager.addAnimal(rabbit);
//
//    // Assert
//    EXPECT_TRUE(result);
//}

TEST(ZooManagerTest, AddAnimal_SickAnimal_ReturnsFalse) {
    // Arrange
    auto mockClinic = std::make_shared<MockVeterinaryClinic>();
    auto mockRepo = std::make_shared<MockZooRepository>();
    
    auto rabbit = std::make_shared<Rabbit>("Sick", 1, 2, 5, false);
    
    EXPECT_CALL(*mockClinic, checkHealth(testing::_))
        .WillOnce(testing::Return(false));
    
    // addAnimal НЕ должен вызываться для больного животного
    EXPECT_CALL(*mockRepo, addAnimal(testing::_))
        .Times(0);
    
    ZooManager manager(mockClinic, mockRepo);
    
    // Act
    bool result = manager.addAnimal(rabbit);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST(ZooManagerTest, AddAnimal_NullPointer_ThrowsException) {
    // Arrange
    auto clinic = std::make_shared<VeterinaryClinic>();
    auto repo = std::make_shared<ZooRepository>();
    ZooManager manager(clinic, repo);
    
    // Act & Assert
    EXPECT_THROW(manager.addAnimal(nullptr), std::invalid_argument);
}

// ========== Интеграционные тесты (с реальными объектами) ==========

TEST(ZooManagerTest, Integration_AddMultipleAnimals_AllStored) {
    // Arrange
    auto clinic = std::make_shared<VeterinaryClinic>();
    auto repo = std::make_shared<ZooRepository>();
    ZooManager manager(clinic, repo);
    
    auto rabbit = std::make_shared<Rabbit>("Bugs", 1, 2, 5);
    auto tiger = std::make_shared<Tiger>("Tony", 2, 15);
    
    // Act
    manager.addAnimal(rabbit);
    manager.addAnimal(tiger);
    
    // Assert
    EXPECT_EQ(manager.getAnimalCount(), 2);
}

TEST(ZooManagerTest, Integration_CalculateTotalFood_ReturnsCorrectSum) {
    // Arrange
    auto clinic = std::make_shared<VeterinaryClinic>();
    auto repo = std::make_shared<ZooRepository>();
    ZooManager manager(clinic, repo);
    
    manager.addAnimal(std::make_shared<Rabbit>("Bunny", 1, 2, 5));    // 2 kg
    manager.addAnimal(std::make_shared<Tiger>("Tiger", 2, 15));        // 15 kg
    manager.addAnimal(std::make_shared<Monkey>("George", 3, 5, 8));    // 5 kg
    
    // Act
    int totalFood = manager.calculateTotalFood();
    
    // Assert
    EXPECT_EQ(totalFood, 22);  // 2 + 15 + 5 = 22
}

TEST(ZooManagerTest, Integration_GetInteractiveAnimals_ReturnsOnlyHerbivoresWithHighKindness) {
    // Arrange
    auto clinic = std::make_shared<VeterinaryClinic>();
  auto repo = std::make_shared<ZooRepository>();
  ZooManager manager(clinic, repo);
  // Добавляем разных животных
  manager.addAnimal(std::make_shared<Rabbit>("Friendly", 1, 2, 7));   // Интерактивный
  manager.addAnimal(std::make_shared<Rabbit>("Grumpy", 2, 2, 3));     // Не интерактивный
  manager.addAnimal(std::make_shared<Monkey>("Happy", 3, 5, 9));      // Интерактивный
  manager.addAnimal(std::make_shared<Tiger>("Fierce", 4, 15));        // Хищник - не интерактивный

  // Act
  auto interactiveAnimals = manager.getInteractiveAnimals();

  // Assert
  EXPECT_EQ(interactiveAnimals.size(), 2);  // Только Friendly и Happy
  EXPECT_EQ(interactiveAnimals[0]->getName(), "Friendly");
  EXPECT_EQ(interactiveAnimals[1]->getName(), "Happy");
}
TEST(ZooManagerTest, Integration_AddThing_IncreasesCount) {
  // Arrange
  auto clinic = std::make_shared<VeterinaryClinic>();
  auto repo = std::make_shared<ZooRepository>();
  ZooManager manager(clinic, repo);
  auto table = std::make_shared<Table>("Office Desk", 100);

  // Act
  manager.addThing(table);

  // Assert
  EXPECT_EQ(manager.getThingCount(), 1);
}
TEST(ZooManagerTest, Integration_MixedInventory_BothAnimalsAndThings) {
  // Arrange
  auto clinic = std::make_shared<VeterinaryClinic>();
  auto repo = std::make_shared<ZooRepository>();
  ZooManager manager(clinic, repo);
  // Act
  manager.addAnimal(std::make_shared<Rabbit>("Bugs", 1, 2, 5));
  manager.addThing(std::make_shared<Table>("Desk", 100));
  manager.addAnimal(std::make_shared<Tiger>("Tony", 2, 15));

  // Assert
  EXPECT_EQ(manager.getAnimalCount(), 2);
  EXPECT_EQ(manager.getThingCount(), 1);
}