#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/VeterinaryClinic.h"
#include "domain/Rabbit.h"

// Мок для IHealthCheckable (для изолированного тестирования)
class MockHealthCheckable : public IHealthCheckable {
public:
    MOCK_METHOD(bool, isHealthy, (), (const, override));
};

// ========== Тесты с реальными объектами ==========

TEST(VeterinaryClinicTest, CheckHealth_HealthyAnimal_ReturnsTrue) {
    // Arrange
    VeterinaryClinic clinic;
    Rabbit rabbit("Healthy Bunny", 1, 2, 5, true);
    
    // Act
    bool result = clinic.checkHealth(rabbit);
    
    // Assert
    EXPECT_TRUE(result);
}

TEST(VeterinaryClinicTest, CheckHealth_SickAnimal_ReturnsFalse) {
    // Arrange
    VeterinaryClinic clinic;
    Rabbit rabbit("Sick Bunny", 2, 2, 5, false);
    
    // Act
    bool result = clinic.checkHealth(rabbit);
    
    // Assert
    EXPECT_FALSE(result);
}

// ========== Тесты с моками ==========

TEST(VeterinaryClinicTest, CheckHealth_MockHealthyAnimal_ReturnsTrue) {
    // Arrange
    VeterinaryClinic clinic;
    MockHealthCheckable mockAnimal;
    
    // Настраиваем мок: ожидаем вызов isHealthy(), вернёт true
    EXPECT_CALL(mockAnimal, isHealthy())
        .WillOnce(testing::Return(true));
    
    // Act
    bool result = clinic.checkHealth(mockAnimal);
    
    // Assert
    EXPECT_TRUE(result);
}

TEST(VeterinaryClinicTest, CheckHealth_MockSickAnimal_ReturnsFalse) {
    // Arrange
    VeterinaryClinic clinic;
    MockHealthCheckable mockAnimal;
    
    EXPECT_CALL(mockAnimal, isHealthy())
        .WillOnce(testing::Return(false));
    
    // Act
    bool result = clinic.checkHealth(mockAnimal);
    
    // Assert
    EXPECT_FALSE(result);
}

// ========== Тест вызова метода ==========

TEST(VeterinaryClinicTest, CheckHealth_CallsIsHealthyMethod) {
    // Arrange
    VeterinaryClinic clinic;
    MockHealthCheckable mockAnimal;
    
    // Проверяем, что метод isHealthy() будет вызван ровно 1 раз
    EXPECT_CALL(mockAnimal, isHealthy())
        .Times(1)
        .WillOnce(testing::Return(true));
    
    // Act
    clinic.checkHealth(mockAnimal);
    
    // Assert - автоматически проверится через EXPECT_CALL
}