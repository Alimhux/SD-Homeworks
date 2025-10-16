#include <gtest/gtest.h>
#include "domain/Rabbit.h"
#include "domain/Monkey.h"

// ========== Тесты Herbivore ==========

TEST(HerbivoreTest, Constructor_ValidKindness_Success) {
    // Arrange & Act
    Rabbit rabbit("Fluffy", 1, 2, 7);

    // Assert
    EXPECT_EQ(rabbit.getKindness(), 7);
    EXPECT_EQ(rabbit.getType(), "Rabbit");
}

TEST(HerbivoreTest, Constructor_KindnessAbove10_ThrowsException) {
    // Arrange, Act & Assert
    EXPECT_THROW(
        Rabbit("Fluffy", 1, 2, 11),
        std::invalid_argument
    );
}

TEST(HerbivoreTest, Constructor_KindnessBelow0_ThrowsException) {
    EXPECT_THROW(
        Rabbit("Fluffy", 1, 2, -1),
        std::invalid_argument
    );
}

TEST(HerbivoreTest, IsInteractive_KindnessAbove5_ReturnsTrue) {
    // Arrange
    Rabbit rabbit("Bugs", 1, 2, 6);

    // Act & Assert
    EXPECT_TRUE(rabbit.isInteractive());
}

TEST(HerbivoreTest, IsInteractive_KindnessEquals5_ReturnsTrue) {
    // Arrange
    Rabbit rabbit("Bugs", 1, 2, 5);

    // Act & Assert
    EXPECT_TRUE(rabbit.isInteractive());
}

TEST(HerbivoreTest, IsInteractive_KindnessBelow5_ReturnsFalse) {
    // Arrange
    Rabbit rabbit("Grumpy", 1, 2, 4);

    // Act & Assert
    EXPECT_FALSE(rabbit.isInteractive());
}

TEST(HerbivoreTest, SetKindness_ValidValue_UpdatesKindness) {
    // Arrange
    Rabbit rabbit("Fluffy", 1, 2, 5);

    // Act
    rabbit.setKindness(8);

    // Assert
    EXPECT_EQ(rabbit.getKindness(), 8);
    EXPECT_TRUE(rabbit.isInteractive());
}

TEST(HerbivoreTest, SetKindness_InvalidValue_ThrowsException) {
    // Arrange
    Rabbit rabbit("Fluffy", 1, 2, 5);

    // Act & Assert
    EXPECT_THROW(rabbit.setKindness(15), std::invalid_argument);
}

TEST(HerbivoreTest, GetInfo_ContainsKindnessInfo) {
    // Arrange
    Rabbit rabbit("Fluffy", 1, 2, 7);

    // Act
    std::string info = rabbit.getInfo();

    // Assert
    EXPECT_NE(info.find("Kindness: 7"), std::string::npos);
    EXPECT_NE(info.find("Interactive: Yes"), std::string::npos);
}


// ========== Тесты конкретных классов ==========

TEST(RabbitTest, GetType_ReturnsRabbit) {
  // Arrange
  Rabbit rabbit("Bugs", 1, 2, 6);

  // Act & Assert
  EXPECT_EQ(rabbit.getType(), "Rabbit");
}

TEST(MonkeyTest, GetType_ReturnsMonkey) {
  // Arrange
  Monkey monkey("George", 2, 5, 8);

  // Act & Assert
  EXPECT_EQ(monkey.getType(), "Monkey");
}

TEST(MonkeyTest, IsInteractive_HighKindness_ReturnsTrue) {
  // Arrange
  Monkey monkey("Curious", 2, 5, 9);

  // Act & Assert
  EXPECT_TRUE(monkey.isInteractive());
}