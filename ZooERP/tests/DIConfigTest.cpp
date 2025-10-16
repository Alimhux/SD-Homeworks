#include <gtest/gtest.h>
#include "infrastructure/DIConfig.h"
#include "infrastructure/DIContainer.h"
#include "domain/Rabbit.h"

TEST(DIConfigTest, Configure_RegistersServices) {
  // Arrange
  DIContainer container;

  // Act
  DIConfig::configure(container);

  // Assert - проверяем, что сервисы зарегистрированы и можно их разрешить
  EXPECT_NO_THROW(container.resolve<IVeterinaryClinic>());
  EXPECT_NO_THROW(container.resolve<IZooRepository>());
}

TEST(DIConfigTest, Configure_VeterinaryClinic_IsSingleton) {
  // Arrange
  DIContainer container;
  DIConfig::configure(container);

  // Act
  auto clinic1 = container.resolve<IVeterinaryClinic>();
  auto clinic2 = container.resolve<IVeterinaryClinic>();

  // Assert - должен быть тот же экземпляр
  EXPECT_EQ(clinic1.get(), clinic2.get());
}

TEST(DIConfigTest, Configure_Repository_IsSingleton) {
  // Arrange
  DIContainer container;
  DIConfig::configure(container);

  // Act
  auto repo1 = container.resolve<IZooRepository>();
  auto repo2 = container.resolve<IZooRepository>();

  // Assert
  EXPECT_EQ(repo1.get(), repo2.get());
}

TEST(DIConfigTest, CreateZooManager_ReturnsFunctionalManager) {
  // Arrange
  DIContainer container;
  DIConfig::configure(container);

  // Act
  auto manager = DIConfig::createZooManager(container);

  // Assert
  EXPECT_NE(manager, nullptr);
  EXPECT_EQ(manager->getAnimalCount(), 0);  // Пустой зоопарк
}

TEST(DIConfigTest, CreateZooManager_WithAddedAnimals_Works) {
  // Arrange
  DIContainer container;
  DIConfig::configure(container);
  auto manager = DIConfig::createZooManager(container);

  auto rabbit = std::make_shared<Rabbit>("Test", 1, 2, 5);

  // Act
  bool added = manager->addAnimal(rabbit);

  // Assert
  EXPECT_TRUE(added);
  EXPECT_EQ(manager->getAnimalCount(), 1);
}