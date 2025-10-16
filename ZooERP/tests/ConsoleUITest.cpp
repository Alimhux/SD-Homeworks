#include <gtest/gtest.h>
#include "ui/ConsoleUI.h"
#include "services/ZooManager.h"
#include "services/VeterinaryClinic.h"
#include "services/ZooRepository.h"

TEST(ConsoleUITest, Constructor_ValidManager_Success) {
  // Arrange
  auto clinic = std::make_shared<VeterinaryClinic>();
  auto repo = std::make_shared<ZooRepository>();
  auto manager = std::make_shared<ZooManager>(clinic, repo);

  // Act & Assert
  EXPECT_NO_THROW(ConsoleUI ui(manager));
}

TEST(ConsoleUITest, Constructor_NullManager_ThrowsException) {
  // Arrange, Act & Assert
  EXPECT_THROW(ConsoleUI ui(nullptr), std::invalid_argument);
}