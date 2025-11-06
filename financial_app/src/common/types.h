#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace financial {

using Id = std::string;
using DateTime = std::chrono::system_clock::time_point;
using Decimal = double;

template <typename T>
class Entity {
 protected:
  Id id_;

 public:
  explicit Entity(const Id& id) : id_(id) {}
  virtual ~Entity() = default;

  const Id& getId() const { return id_; }

  bool operator==(const Entity& other) const { return id_ == other.id_; }

  bool operator!=(const Entity& other) const { return !(*this == other); }
};

// Base Value Object
class ValueObject {
 public:
  virtual ~ValueObject() = default;
  virtual bool equals(const ValueObject& other) const = 0;
};

template <typename T, typename E>
class Result {
 private:
  std::variant<T, E> value_;

 public:
  static Result<T, E> success(const T& value) { return Result<T, E>(value); }

  static Result<T, E> failure(const E& error) { return Result<T, E>(error); }

  bool isSuccess() const { return std::holds_alternative<T>(value_); }

  bool isFailure() const { return std::holds_alternative<E>(value_); }

  const T& getValue() const { return std::get<T>(value_); }

  const E& getError() const { return std::get<E>(value_); }

 private:
  explicit Result(const T& value) : value_(value) {}
  explicit Result(const E& error) : value_(error) {}
};

}  // namespace financial