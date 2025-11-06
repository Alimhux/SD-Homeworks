#pragma once

#include "common/types.h"
#include "common/validation.h"
#include "common/utils.h"
#include "domain/value_objects/types.h"

namespace financial::domain {

// Категория - представляет собой категорию трат или доходов
class Category : public Entity<Category> {
private:
    CategoryType type_;
    std::string name_;
    std::string description_;
    std::string color_; // для UI
    std::string icon_;
    
public:
    Category(const Id& id, CategoryType type, const std::string& name,
             const std::string& description = "", 
             const std::string& color = "#000000",
             const std::string& icon = "default")
        : Entity(id), type_(type), name_(name), 
          description_(description), color_(color), icon_(icon) {
        validate();
    }
    
    // геттеры
    CategoryType getType() const { return type_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getColor() const { return color_; }
    const std::string& getIcon() const { return icon_; }
    
    // сеттеры
    void setName(const std::string& name) {
        Validator::validateNotEmpty(name, "Category name");
        Validator::validateMaxLength(name, 50, "Category name");
        name_ = name;
    }
    
    void setDescription(const std::string& description) {
        Validator::validateMaxLength(description, 200, "Category description");
        description_ = description;
    }
    
    void setColor(const std::string& color) {
        Validator::validateColor(color_);
        color_ = color;
    }
    
    void setIcon(const std::string& icon) {
        icon_ = icon;
    }
    
    // Business logic methods
    bool isIncomeCategory() const {
        return type_ == CategoryType::INCOME;
    }
    
    bool isExpenseCategory() const {
        return type_ == CategoryType::EXPENSE;
    }
    
    // Метод для создания дефолтной категории дохода
    static Category createDefaultIncomeCategory(const std::string& name) {
        return Category(
            IdGenerator::generate("CAT"), CategoryType::INCOME, name,
            "Default income category",
            "#4CAF50",  // Green
            "income"
        );
    }
    
    static Category createDefaultExpenseCategory(const std::string& name) {
        return Category(
            IdGenerator::generate("CAT"),
            CategoryType::EXPENSE,
            name,
            "Default expense category",
            "#F44336",  // Red
            "expense"
        );
    }
    
private:
    void validate() {
        Validator::validateId(id_);
        Validator::validateColor(color_);
        Validator::validateNotEmpty(name_, "Category name");
        Validator::validateMaxLength(name_, 50, "Category name");
        Validator::validateMaxLength(description_, 200, "Category description");
    }
};

} // namespace financial::domain