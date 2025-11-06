#pragma once

#include <stdexcept>
#include <string>

namespace financial {

    // Базовый класс ошибок для проекта
    class FinancialException : public std::runtime_error {
    public:
        explicit FinancialException(const std::string& message)
            : std::runtime_error(message) {}
    };

    // Ошибки доменной области
    class DomainException : public FinancialException {
        public:
            explicit DomainException(const std::string& message)
            : FinancialException("Domain Error: " + message) {}
    };


    class ValidationException : public DomainException {
    public:
        explicit ValidationException(const std::string& message)
            : DomainException("Validation failed: " + message) {}
    };

    class EntityNotFoundException : public DomainException {
    public:
        explicit EntityNotFoundException(const std::string& entityType, const std::string& id)
            : DomainException(entityType + " with ID '" + id + "' not found") {}
    };

    class InsufficientFundsException : public DomainException {
    public:
        explicit InsufficientFundsException(double requested, double available)
            : DomainException("Insufficient funds. Requested: " + std::to_string(requested) +
                              ", Available: " + std::to_string(available)) {}
    };

    // Ошибки в инфраструктуре
    class InfrastructureException : public FinancialException {
    public:
        explicit InfrastructureException(const std::string& message)
            : FinancialException("Infrastructure Error: " + message) {}
    };

    class PersistenceException : public InfrastructureException {
    public:
        explicit PersistenceException(const std::string& message)
            : InfrastructureException("Persistence failed: " + message) {}
    };

    class SerializationException : public InfrastructureException {
    public:
        explicit SerializationException(const std::string& message)
            : InfrastructureException("Serialization failed: " + message) {}
    };

}