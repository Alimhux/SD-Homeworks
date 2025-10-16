#include <gtest/gtest.h>
#include "domain/Table.h"
#include "domain/Computer.h"

// ========== Тесты Thing ==========

TEST(ThingTest, Constructor_ValidData_Success) {
  // Arrange & Act
  Table table("Office Desk", 100);

  // Assert
  EXPECT_EQ(table.getName(), "Office Desk");
  EXPECT_EQ(table.getInventoryNumber(), 100);
  EXPECT_EQ(table.getType(), "Table");
}

TEST(ThingTest, Constructor_InvalidInventoryNumber_ThrowsException) {
  // Arrange, Act & Assert
  EXPECT_THROW(
      Table("Desk", -1),
      std::invalid_argument
  );
}

TEST(ThingTest, Constructor_EmptyName_ThrowsException) {
  // Arrange, Act & Assert
  EXPECT_THROW(
      Table("", 100),
      std::invalid_argument
  );
}

TEST(ThingTest, SetInventoryNumber_ValidNumber_Updates) {
  // Arrange
  Table table("Desk", 100);

  // Act
  table.setInventoryNumber(200);

  // Assert
  EXPECT_EQ(table.getInventoryNumber(), 200);
}

TEST(ThingTest, GetInfo_ContainsCorrectInfo) {
  // Arrange
  Table table("Conference Table", 101);

  // Act
  std::string info = table.getInfo();

  // Assert
  EXPECT_NE(info.find("Conference Table"), std::string::npos);
  EXPECT_NE(info.find("Table"), std::string::npos);
  EXPECT_NE(info.find("101"), std::string::npos);
}


TEST(TableTest, GetType_ReturnsTable) {
  // Arrange
  Table table("Dining Table", 102);

  // Act & Assert
  EXPECT_EQ(table.getType(), "Table");
}

// ========== Тесты Computer ==========

TEST(ComputerTest, Constructor_ValidData_Success) {
  // Arrange & Act
  Computer computer("Dell Workstation", 200);

  // Assert
  EXPECT_EQ(computer.getName(), "Dell Workstation");
  EXPECT_EQ(computer.getInventoryNumber(), 200);
  EXPECT_EQ(computer.getType(), "Computer");
}

TEST(ComputerTest, GetType_ReturnsComputer) {
  // Arrange
  Computer computer("MacBook Pro", 201);

  // Act & Assert
  EXPECT_EQ(computer.getType(), "Computer");
}

// ========== Тесты полиморфизма ==========

TEST(ThingTest, Polymorphism_ThingPointer_WorksCorrectly) {
  // Arrange - используем указатель на базовый класс
  std::shared_ptr<Thing> thing = std::make_shared<Computer>("Laptop", 300);

  // Act & Assert
  EXPECT_EQ(thing->getType(), "Computer");
  EXPECT_EQ(thing->getInventoryNumber(), 300);
}

TEST(ThingTest, Polymorphism_MultipleThings_AllWork) {
  // Arrange
  std::vector<std::shared_ptr<Thing>> things;
  things.push_back(std::make_shared<Table>("Table1", 1));
  things.push_back(std::make_shared<Computer>("PC1", 2));

  // Act & Assert
  EXPECT_EQ(things.size(), 2);
  EXPECT_EQ(things[0]->getType(), "Table");
  EXPECT_EQ(things[1]->getType(), "Computer");
}