#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "domain/entities/bank_account.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"

namespace financial::infrastructure {

using namespace financial::domain;

// Предварительные объявления для паттерна посетитель
class CSVExportVisitor;
class JSONExportVisitor;

// Интерфейс посетителя
class IExportVisitor {
 public:
  virtual ~IExportVisitor() = default;

  virtual void visit(const BankAccount& account) = 0;
  virtual void visit(const Category& category) = 0;
  virtual void visit(const Operation& operation) = 0;

  virtual std::string getResult() const = 0;
  virtual void reset() = 0;
};

// Интерфейс элемента для посещаемых объектов
class IExportable {
 public:
  virtual ~IExportable() = default;
  virtual void accept(IExportVisitor& visitor) const = 0;
};

// Классы-обёртки для добавления доменных сущностей в посещаемые
class ExportableBankAccount : public IExportable {
 private:
  const BankAccount& account_;

 public:
  explicit ExportableBankAccount(const BankAccount& account)
      : account_(account) {}

  void accept(IExportVisitor& visitor) const override {
    visitor.visit(account_);
  }
};

class ExportableCategory : public IExportable {
 private:
  const Category& category_;

 public:
  explicit ExportableCategory(const Category& category) : category_(category) {}

  void accept(IExportVisitor& visitor) const override {
    visitor.visit(category_);
  }
};

class ExportableOperation : public IExportable {
 private:
  const Operation& operation_;

 public:
  explicit ExportableOperation(const Operation& operation)
      : operation_(operation) {}

  void accept(IExportVisitor& visitor) const override {
    visitor.visit(operation_);
  }
};

// CSV Экспортёр
class CSVExportVisitor : public IExportVisitor {
private:
    std::stringstream buffer_;
    bool firstAccount_ = true;
    bool firstCategory_ = true;
    bool firstOperation_ = true;

public:
    void visit(const BankAccount& account) override {
        if (firstAccount_) {
            buffer_ << "[ACCOUNTS]\n";
            buffer_ << "id,name,balance,currency,accountNumber,isActive\n";
            firstAccount_ = false;
        }

        // Экранирование запятых в строках
        std::string name = escapeCSV(account.getName());
        buffer_ << account.getId() << ","
                << name << ","
                << account.getBalance().getAmount() << ","
                << account.getCurrency() << ","
                << account.getAccountNumber() << ","
                << (account.getIsActive() ? "true" : "false") << "\n";
    }

    void visit(const Category& category) override {
        if (firstCategory_) {
            buffer_ << "\n[CATEGORIES]\n";
            buffer_ << "id,type,name,description\n";
            firstCategory_ = false;
        }

        // Экранирование запятых в строках
        std::string name = escapeCSV(category.getName());
        std::string description = escapeCSV(category.getDescription());
        buffer_ << category.getId() << ","
                << categoryTypeToString(category.getType()) << ","
                << name << ","
                << description << "\n";
    }

    void visit(const Operation& operation) override {
        if (firstOperation_) {
            buffer_ << "\n[OPERATIONS]\n";
            buffer_ << "id,type,bankAccountId,amount,currency,date,categoryId,description\n";
            firstOperation_ = false;
        }

        // Экранирование запятых в строках
        std::string description = escapeCSV(operation.getDescription());
        buffer_ << operation.getId() << ","
                << operationTypeToString(operation.getType()) << ","
                << operation.getBankAccountId() << ","
                << operation.getAmount().getAmount() << ","
                << operation.getAmount().getCurrency() << ","
                << DateTimeUtils::toString(operation.getDate()) << ","
                << operation.getCategoryId() << ","
                << description << "\n";
    }

    // Метод для экранирования запятых и кавычек в CSV
    std::string escapeCSV(const std::string& input) const {
        if (input.find(',') != std::string::npos ||
            input.find('\"') != std::string::npos) {
            std::string output = "\"";
            for (char c : input) {
                if (c == '\"') {
                    output += "\"\"";
                } else {
                    output += c;
                }
            }
            output += "\"";
            return output;
        }
        return input;
    }

    std::string getResult() const override {
        // Добавляем BOM (0xEF, 0xBB, 0xBF) для правильного распознавания UTF-8 в Excel
        return "\xEF\xBB\xBF" + buffer_.str();
    }

    void reset() override {
        buffer_.str("");
        buffer_.clear();
        firstAccount_ = true;
        firstCategory_ = true;
        firstOperation_ = true;
    }
};

// Посетитель экспорта в JSON
class JSONExportVisitor : public IExportVisitor {
  std::stringstream buffer_;
  std::vector<std::string> accounts_;
  std::vector<std::string> categories_;
  std::vector<std::string> operations_;

 public:
  void visit(const BankAccount& account) override {
    std::stringstream ss;
    ss << "    {\n"
       << "      \"id\": \"" << account.getId() << "\",\n"
       << "      \"name\": \"" << account.getName() << "\",\n"
       << "      \"balance\": " << account.getBalance().getAmount() << ",\n"
       << "      \"currency\": \"" << account.getCurrency() << "\",\n"
       << "      \"accountNumber\": \"" << account.getAccountNumber() << "\",\n"
       << "      \"isActive\": " << (account.getIsActive() ? "true" : "false")
       << "\n"
       << "    }";
    accounts_.push_back(ss.str());
  }

  void visit(const Category& category) override {
    std::stringstream ss;
    ss << "    {\n"
       << "      \"id\": \"" << category.getId() << "\",\n"
       << "      \"type\": \"" << categoryTypeToString(category.getType())
       << "\",\n"
       << "      \"name\": \"" << category.getName() << "\",\n"
       << "      \"description\": \"" << category.getDescription() << "\"\n"
       << "    }";
    categories_.push_back(ss.str());
  }

  void visit(const Operation& operation) override {
    std::stringstream ss;
    ss << "    {\n"
       << "      \"id\": \"" << operation.getId() << "\",\n"
       << "      \"type\": \"" << operationTypeToString(operation.getType())
       << "\",\n"
       << "      \"bankAccountId\": \"" << operation.getBankAccountId()
       << "\",\n"
       << "      \"amount\": " << operation.getAmount().getAmount() << ",\n"
       << "      \"currency\": \"" << operation.getAmount().getCurrency()
       << "\",\n"
       << "      \"date\": \"" << DateTimeUtils::toString(operation.getDate())
       << "\",\n"
       << "      \"categoryId\": \"" << operation.getCategoryId() << "\",\n"
       << "      \"description\": \"" << operation.getDescription() << "\"\n"
       << "    }";
    operations_.push_back(ss.str());
  }

  std::string getResult() const override {
    std::stringstream result;
    result << "{\n";

    // Счета
    result << "  \"accounts\": [\n";
    for (size_t i = 0; i < accounts_.size(); ++i) {
      result << accounts_[i];
      if (i < accounts_.size() - 1) result << ",";
      result << "\n";
    }
    result << "  ],\n";

    // Категории
    result << "  \"categories\": [\n";
    for (size_t i = 0; i < categories_.size(); ++i) {
      result << categories_[i];
      if (i < categories_.size() - 1) result << ",";
      result << "\n";
    }
    result << "  ],\n";

    // Операции
    result << "  \"operations\": [\n";
    for (size_t i = 0; i < operations_.size(); ++i) {
      result << operations_[i];
      if (i < operations_.size() - 1) result << ",";
      result << "\n";
    }
    result << "  ]\n";

    result << "}";
    return result.str();
  }

  void reset() override {
    buffer_.str("");
    buffer_.clear();
    accounts_.clear();
    categories_.clear();
    operations_.clear();
  }
};

// Экспортер данных с использованием паттерна посетитель
class DataExporter {
 private:
  std::unique_ptr<IExportVisitor> visitor_;

 public:
  explicit DataExporter(std::unique_ptr<IExportVisitor> visitor)
      : visitor_(std::move(visitor)) {}

  void exportToFile(const std::string& filename,
                    const std::vector<std::shared_ptr<BankAccount>>& accounts,
                    const std::vector<std::shared_ptr<Category>>& categories,
                    const std::vector<std::shared_ptr<Operation>>& operations) {
    visitor_->reset();

    // Посетить все счета
    for (const auto& account : accounts) {
      ExportableBankAccount exportable(*account);
      exportable.accept(*visitor_);
    }

    // Посетить все категории
    for (const auto& category : categories) {
      ExportableCategory exportable(*category);
      exportable.accept(*visitor_);
    }

    // Посетить все операции
    for (const auto& operation : operations) {
      ExportableOperation exportable(*operation);
      exportable.accept(*visitor_);
    }

    // Записать в файл
    std::ofstream file(filename);
    if (!file.is_open()) {
      throw InfrastructureException("Невозможно создать файл: " + filename);
    }

    file << visitor_->getResult();
    file.close();
  }

  std::string exportToString(
      const std::vector<std::shared_ptr<BankAccount>>& accounts,
      const std::vector<std::shared_ptr<Category>>& categories,
      const std::vector<std::shared_ptr<Operation>>& operations) {
    visitor_->reset();

    for (const auto& account : accounts) {
      ExportableBankAccount exportable(*account);
      exportable.accept(*visitor_);
    }

    for (const auto& category : categories) {
      ExportableCategory exportable(*category);
      exportable.accept(*visitor_);
    }

    for (const auto& operation : operations) {
      ExportableOperation exportable(*operation);
      exportable.accept(*visitor_);
    }

    return visitor_->getResult();
  }
};

// Фабрика для создания экспортеров
class ExporterFactory {
 public:
  static std::unique_ptr<DataExporter> create(const std::string& format) {
    if (format == "csv" || format == "CSV") {
      return std::make_unique<DataExporter>(
          std::make_unique<CSVExportVisitor>());
    }
    if (format == "json" || format == "JSON") {
      return std::make_unique<DataExporter>(
          std::make_unique<JSONExportVisitor>());
    }
    throw std::invalid_argument("Неподдерживаемый формат экспорта: " + format);
  }
};

}  // namespace financial::infrastructure