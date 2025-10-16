#include <gtest/gtest.h>
#include "infrastructure/DIContainer.h"

// Простой тестовый интерфейс
class ITestService {
public:
  virtual ~ITestService() = default;
  virtual int getValue() const = 0;
};

// Реализация
class TestService : public ITestService {
private:
  int value_;
public:
  explicit TestService(int value = 42) : value_(value) {}
  int getValue() const override { return value_; }
};

// Тест: Регистрация и разрешение Singleton
TEST(DIContainerTest, RegisterSingleton_ResolveTwice_ReturnsSameInstance) {
  // Arrange
  DIContainer container;
  container.registerSingleton<ITestService, TestService>();

  // Act
  auto instance1 = container.resolve<ITestService>();
  auto instance2 = container.resolve<ITestService>();

  // Assert
  EXPECT_EQ(instance1.get(), instance2.get()); // Тот же указатель
  EXPECT_EQ(instance1->getValue(), 42);
}

// Тест: Transient создаёт новые экземпляры
TEST(DIContainerTest, RegisterTransient_ResolveTwice_ReturnsDifferentInstances) {
  // Arrange
  DIContainer container;
  container.registerTransient<ITestService, TestService>();

  // Act
  auto instance1 = container.resolve<ITestService>();
  auto instance2 = container.resolve<ITestService>();

  // Assert
  EXPECT_NE(instance1.get(), instance2.get()); // Разные указатели
}

// Тест: Исключение при незарегистрированном типе
TEST(DIContainerTest, Resolve_UnregisteredType_ThrowsException) {
  // Arrange
  DIContainer container;

  // Act & Assert
  EXPECT_THROW(container.resolve<ITestService>(), std::runtime_error);
}

// Тест: Singleton с параметрами конструктора
TEST(DIContainerTest, RegisterSingleton_WithConstructorArgs_UsesArgs) {
  // Arrange
  DIContainer container;
  container.registerSingleton<ITestService, TestService>(100);

  // Act
  auto instance = container.resolve<ITestService>();

  // Assert
  EXPECT_EQ(instance->getValue(), 100);
}