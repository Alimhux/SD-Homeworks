#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "common/exceptions.h"

namespace financial::infrastructure {
using namespace financial::domain;

// Объекты передачи данных для импорта/экспорта
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

// Шаблонный метод для импорта данных
class DataImporter {
 protected:
  // Шаблонный метод
 public:
  virtual ~DataImporter() = default;

  ImportData import(const std::string& filename) {
    // Открыть файл
    std::ifstream file = openFile(filename);

    // Прочитать необработанное содержимое
    std::string content = readContent(file);

    //  Разобрать содержимое (реализуется подклассами)
    ImportData data = parseContent(content);

    // Проверить данные
    validateData(data);

    // Закрыть файл
    closeFile(file);

    return data;
  }

 protected:
  // Общие шаги
  virtual std::ifstream openFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw InfrastructureException("Невозможно открыть файл: " + filename);
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
    // Базовая проверка
    for (const auto& account : data.accounts) {
      if (account.id.empty() || account.name.empty()) {
        throw ValidationException("Неверные данные счёта");
      }
    }

    for (const auto& category : data.categories) {
      if (category.id.empty() || category.name.empty()) {
        throw ValidationException("Неверные данные категории");
      }
    }

    for (const auto& operation : data.operations) {
      if (operation.id.empty() || operation.bankAccountId.empty() ||
          operation.categoryId.empty() || operation.amount <= 0) {
        throw ValidationException("Неверные данные операции");
      }
    }
  }

  virtual ImportData parseContent(const std::string& content) = 0;
};

// Импортер JSON  - ручной парсинг
class JSONImporter : public DataImporter {
 public:
  ~JSONImporter() override = default;

 protected:
  ImportData parseContent(const std::string& content) override {
    ImportData data;

    // Парсим accounts
    size_t accountsPos = content.find("\"accounts\"");
    if (accountsPos != std::string::npos) {
      size_t arrayStart = content.find('[', accountsPos);
      size_t arrayEnd = content.find(']', arrayStart);
      if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
        std::string accountsSection =
            content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        parseAccountsArray(accountsSection, data.accounts);
      }
    }

    // Парсим categories
    size_t categoriesPos = content.find("\"categories\"");
    if (categoriesPos != std::string::npos) {
      size_t arrayStart = content.find('[', categoriesPos);
      size_t arrayEnd = content.find(']', arrayStart);
      if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
        std::string categoriesSection =
            content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        parseCategoriesArray(categoriesSection, data.categories);
      }
    }

    size_t operationsPos = content.find("\"operations\"");
    if (operationsPos != std::string::npos) {
      size_t arrayStart = content.find('[', operationsPos);
      size_t arrayEnd = content.find(']', arrayStart);
      if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
        std::string operationsSection =
            content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        parseOperationsArray(operationsSection, data.operations);
      }
    }

    return data;
  }

 private:
  std::string extractString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";

    size_t valueStart = json.find('"', pos + searchKey.length());
    if (valueStart == std::string::npos) return "";

    size_t valueEnd = json.find('"', valueStart + 1);
    if (valueEnd == std::string::npos) return "";

    return json.substr(valueStart + 1, valueEnd - valueStart - 1);
  }

  double extractNumber(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return 0.0;

    size_t valueStart = pos + searchKey.length();
    while (valueStart < json.length() && std::isspace(json[valueStart])) {
      valueStart++;
    }

    size_t valueEnd = valueStart;
    while (valueEnd < json.length() &&
           (std::isdigit(json[valueEnd]) || json[valueEnd] == '.' ||
            json[valueEnd] == '-')) {
      valueEnd++;
    }

    if (valueEnd > valueStart) {
      return std::stod(json.substr(valueStart, valueEnd - valueStart));
    }
    return 0.0;
  }

  bool extractBool(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return false;

    size_t valueStart = pos + searchKey.length();
    return json.find("true", valueStart) < json.find("false", valueStart);
  }

  std::vector<std::string> splitObjects(const std::string& arrayContent) {
    std::vector<std::string> objects;
    int braceLevel = 0;
    size_t objectStart = 0;
    bool inString = false;

    for (size_t i = 0; i < arrayContent.length(); i++) {
      char c = arrayContent[i];

      if (c == '"' && (i == 0 || arrayContent[i - 1] != '\\')) {
        inString = !inString;
      }

      if (!inString) {
        if (c == '{') {
          if (braceLevel == 0) {
            objectStart = i;
          }
          braceLevel++;
        } else if (c == '}') {
          braceLevel--;
          if (braceLevel == 0) {
            objects.push_back(
                arrayContent.substr(objectStart, i - objectStart + 1));
          }
        }
      }
    }

    return objects;
  }

  void parseAccountsArray(const std::string& arrayContent,
                          std::vector<AccountDTO>& accounts) {
    auto objects = splitObjects(arrayContent);

    for (const auto& obj : objects) {
      AccountDTO account;
      account.id = extractString(obj, "id");
      account.name = extractString(obj, "name");
      account.balance = extractNumber(obj, "balance");
      account.currency = extractString(obj, "currency");
      account.accountNumber = extractString(obj, "accountNumber");
      account.isActive = extractBool(obj, "isActive");

      if (!account.id.empty()) {
        accounts.push_back(account);
      }
    }
  }

  void parseCategoriesArray(const std::string& arrayContent,
                            std::vector<CategoryDTO>& categories) {
    auto objects = splitObjects(arrayContent);

    for (const auto& obj : objects) {
      CategoryDTO category;
      category.id = extractString(obj, "id");
      category.type = extractString(obj, "type");
      category.name = extractString(obj, "name");
      category.description = extractString(obj, "description");

      if (!category.id.empty()) {
        categories.push_back(category);
      }
    }
  }

  void parseOperationsArray(const std::string& arrayContent,
                            std::vector<OperationDTO>& operations) {
    auto objects = splitObjects(arrayContent);

    for (const auto& obj : objects) {
      OperationDTO operation;
      operation.id = extractString(obj, "id");
      operation.type = extractString(obj, "type");
      operation.bankAccountId = extractString(obj, "bankAccountId");
      operation.amount = extractNumber(obj, "amount");
      operation.currency = extractString(obj, "currency");
      operation.date = extractString(obj, "date");
      operation.categoryId = extractString(obj, "categoryId");
      operation.description = extractString(obj, "description");

      if (!operation.id.empty()) {
        operations.push_back(operation);
      }
    }
  }
};

// Фабрика для создания импортеров
class ImporterFactory {
 public:
  static std::unique_ptr<DataImporter> create(const std::string& format) {
    if (format == "json" || format == "JSON") {
      return std::make_unique<JSONImporter>();
    }
    throw std::invalid_argument("Неподдерживаемый формат импорта: " + format);
  }

  static std::unique_ptr<DataImporter> createFromFilename(
      const std::string& filename) {
    auto extension = filename.substr(filename.find_last_of('.') + 1);
    return create(extension);
  }
};

}  // namespace financial::infrastructure