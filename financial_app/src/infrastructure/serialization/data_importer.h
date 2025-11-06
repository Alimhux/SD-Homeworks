#pragma once

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "common/exceptions.h"
#include "domain/entities/bank_account.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"

namespace financial::infrastructure {

using namespace financial::domain;

// Data transfer objects for import/export
struct AccountDTO {
  std::string id;
  std::string name;
  double balance;
  std::string currency;
  std::string accountNumber;
  bool isActive;
};

struct CategoryDTO {
  std::string id;
  std::string type;
  std::string name;
  std::string description;
};

struct OperationDTO {
  std::string id;
  std::string type;
  std::string bankAccountId;
  double amount;
  std::string currency;
  std::string date;
  std::string categoryId;
  std::string description;
};

struct ImportData {
  std::vector<AccountDTO> accounts;
  std::vector<CategoryDTO> categories;
  std::vector<OperationDTO> operations;
};

// Template Method pattern for data import
class DataImporter {
 protected:
  // Template method
 public:
  virtual ~DataImporter() = default;

  ImportData import(const std::string& filename) {
    // Step 1: Open file
    std::ifstream file = openFile(filename);

    // Step 2: Read raw content
    std::string content = readContent(file);

    // Step 3: Parse content (implemented by subclasses)
    ImportData data = parseContent(content);

    // Step 4: Validate data
    validateData(data);

    // Step 5: Close file
    closeFile(file);

    return data;
  }

 protected:
  // Common steps
  virtual std::ifstream openFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw InfrastructureException("Cannot open file: " + filename);
    }
    return file;
  }

  virtual std::string readContent(std::ifstream& file) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  virtual void closeFile(std::ifstream& file) { file.close(); }

  virtual void validateData(const ImportData& data) {
    // Basic validation
    for (const auto& account : data.accounts) {
      if (account.id.empty() || account.name.empty()) {
        throw ValidationException("Invalid account data");
      }
    }

    for (const auto& category : data.categories) {
      if (category.id.empty() || category.name.empty()) {
        throw ValidationException("Invalid category data");
      }
    }

    for (const auto& operation : data.operations) {
      if (operation.id.empty() || operation.bankAccountId.empty() ||
          operation.categoryId.empty() || operation.amount <= 0) {
        throw ValidationException("Invalid operation data");
      }
    }
  }

  // Abstract method to be implemented by subclasses
  virtual ImportData parseContent(const std::string& content) = 0;
};

// CSV Importer
class CSVImporter : public DataImporter {
 public:
  ~CSVImporter() override = default;

 protected:
  ImportData parseContent(const std::string& content) override {
    ImportData data;
    std::istringstream stream(content);
    std::string line;
    std::string section;

    while (std::getline(stream, line)) {
      if (line.empty()) continue;

      // Check for section headers
      if (line == "[ACCOUNTS]") {
        section = "ACCOUNTS";
        continue;
      } else if (line == "[CATEGORIES]") {
        section = "CATEGORIES";
        continue;
      } else if (line == "[OPERATIONS]") {
        section = "OPERATIONS";
        continue;
      }

      // Parse based on current section
      if (section == "ACCOUNTS") {
        parseAccountLine(line, data.accounts);
      } else if (section == "CATEGORIES") {
        parseCategoryLine(line, data.categories);
      } else if (section == "OPERATIONS") {
        parseOperationLine(line, data.operations);
      }
    }

    return data;
  }

 private:
  std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
      // Remove leading/trailing whitespace
      field.erase(0, field.find_first_not_of(" \t"));
      field.erase(field.find_last_not_of(" \t") + 1);
      result.push_back(field);
    }

    return result;
  }

  void parseAccountLine(const std::string& line,
                        std::vector<AccountDTO>& accounts) {
    auto fields = splitCSV(line);
    if (fields.size() >= 6) {
      AccountDTO account;
      account.id = fields[0];
      account.name = fields[1];
      account.balance = std::stod(fields[2]);
      account.currency = fields[3];
      account.accountNumber = fields[4];
      account.isActive = (fields[5] == "true");
      accounts.push_back(account);
    }
  }

  void parseCategoryLine(const std::string& line,
                         std::vector<CategoryDTO>& categories) {
    auto fields = splitCSV(line);
    if (fields.size() >= 4) {
      CategoryDTO category;
      category.id = fields[0];
      category.type = fields[1];
      category.name = fields[2];
      category.description = fields[3];
      categories.push_back(category);
    }
  }

  void parseOperationLine(const std::string& line,
                          std::vector<OperationDTO>& operations) {
    auto fields = splitCSV(line);
    if (fields.size() >= 8) {
      OperationDTO operation;
      operation.id = fields[0];
      operation.type = fields[1];
      operation.bankAccountId = fields[2];
      operation.amount = std::stod(fields[3]);
      operation.currency = fields[4];
      operation.date = fields[5];
      operation.categoryId = fields[6];
      operation.description = fields[7];
      operations.push_back(operation);
    }
  }
};

// JSON Importer (simplified - in production would use a JSON library)
class JSONImporter : public DataImporter {
 public:
  ~JSONImporter() override = default;

 protected:
  ImportData parseContent(const std::string& content) override {
    ImportData data;
    // Simplified JSON parsing - in production use nlohmann/json or similar
    // This is just a placeholder to demonstrate the pattern

    if (content.find("\"accounts\"") != std::string::npos) {
      // Parse accounts section
    }

    if (content.find("\"categories\"") != std::string::npos) {
      // Parse categories section
    }

    if (content.find("\"operations\"") != std::string::npos) {
      // Parse operations section
    }

    return data;
  }
};

// Factory for creating importers
class ImporterFactory {
 public:
  static std::unique_ptr<DataImporter> create(const std::string& format) {
    if (format == "csv" || format == "CSV") {
      return std::make_unique<CSVImporter>();
    } else if (format == "json" || format == "JSON") {
      return std::make_unique<JSONImporter>();
    } else {
      throw std::invalid_argument("Unsupported import format: " + format);
    }
  }

  static std::unique_ptr<DataImporter> createFromFilename(
      const std::string& filename) {
    auto extension = filename.substr(filename.find_last_of('.') + 1);
    return create(extension);
  }
};

}  // namespace financial::infrastructure