#include <gtest/gtest.h>
#include "domain/Animal.h"

// Конкретная реализация для тестирования
class TestAnimal : public Animal {
public:
    TestAnimal(const std::string& name, int invNumber, int food, bool healthy = true)
        : Animal(name, invNumber, food, healthy) {}

    std::string getType() const override { return "TestAnimal"; }
};

TEST(AnimalTest, Constructor_ValidData_Success) {
    // Arrange & Act
    TestAnimal animal("Leo", 1, 10, true);

    // Assert
    EXPECT_EQ(animal.getName(), "Leo");
    EXPECT_EQ(animal.getInventoryNumber(), 1);
    EXPECT_EQ(animal.getFood(), 10);
    EXPECT_TRUE(animal.isHealthy());
}

// Тест: Исключение при отрицательном номере
TEST(AnimalTest, Constructor_NegativeInventoryNumber_ThrowsException) {
    // Arrange, Act & Assert
    EXPECT_THROW(
        TestAnimal("Leo", -1, 10),
        std::invalid_argument
    );
}

// Тест: Исключение при отрицательном количестве еды
TEST(AnimalTest, Constructor_NegativeFood_ThrowsException) {
    EXPECT_THROW(
        TestAnimal("Leo", 1, -5),
        std::invalid_argument
    );
}

// Тест: Исключение при пустом имени
TEST(AnimalTest, Constructor_EmptyName_ThrowsException) {
    EXPECT_THROW(
        TestAnimal("", 1, 10),
        std::invalid_argument
    );
}

// Тест: Установка количества еды
TEST(AnimalTest, SetFood_ValidAmount_UpdatesFood) {
    // Arrange
    TestAnimal animal("Leo", 1, 10);

    // Act
    animal.setFood(15);

    // Assert
    EXPECT_EQ(animal.getFood(), 15);
}

// Тест: Исключение при установке отрицательной еды
TEST(AnimalTest, SetFood_NegativeAmount_ThrowsException) {
    TestAnimal animal("Leo", 1, 10);

    EXPECT_THROW(animal.setFood(-5), std::invalid_argument);
}

// Тест: GetInfo возвращает корректную информацию
TEST(AnimalTest, GetInfo_ReturnsCorrectString) {
    // Arrange
    TestAnimal animal("Leo", 1, 10, true);

    // Act
    std::string info = animal.getInfo();

    // Assert
    EXPECT_NE(info.find("Leo"), std::string::npos);
    EXPECT_NE(info.find("10 kg"), std::string::npos);
    EXPECT_NE(info.find("Healthy"), std::string::npos);
}