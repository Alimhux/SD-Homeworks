#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include "domain/entities/bank_account.h"
#include "domain/entities/category.h"
#include "domain/entities/operation.h"

namespace financial::infrastructure {

using namespace financial::domain;

// Forward declarations for visitor pattern
class CSVExportVisitor;
class JSONExportVisitor;
class XMLExportVisitor;

// Visitor interface
class IExportVisitor {
public:
    virtual ~IExportVisitor() = default;
    
    virtual void visit(const BankAccount& account) = 0;
    virtual void visit(const Category& category) = 0;
    virtual void visit(const Operation& operation) = 0;
    
    virtual std::string getResult() const = 0;
    virtual void reset() = 0;
};

// Element interface for visitable objects
class IExportable {
public:
    virtual ~IExportable() = default;
    virtual void accept(IExportVisitor& visitor) const = 0;
};

// Wrapper classes to make domain entities visitable
class ExportableBankAccount : public IExportable {
private:
    const BankAccount& account_;
    
public:
    explicit ExportableBankAccount(const BankAccount& account) : account_(account) {}
    
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
    explicit ExportableOperation(const Operation& operation) : operation_(operation) {}
    
    void accept(IExportVisitor& visitor) const override {
        visitor.visit(operation_);
    }
};

// CSV Export Visitor
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
        
        buffer_ << account.getId() << ","
                << account.getName() << ","
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
        
        buffer_ << category.getId() << ","
                << categoryTypeToString(category.getType()) << ","
                << category.getName() << ","
                << category.getDescription() << "\n";
    }
    
    void visit(const Operation& operation) override {
        if (firstOperation_) {
            buffer_ << "\n[OPERATIONS]\n";
            buffer_ << "id,type,bankAccountId,amount,currency,date,categoryId,description\n";
            firstOperation_ = false;
        }
        
        buffer_ << operation.getId() << ","
                << operationTypeToString(operation.getType()) << ","
                << operation.getBankAccountId() << ","
                << operation.getAmount().getAmount() << ","
                << operation.getAmount().getCurrency() << ","
                << DateTimeUtils::toString(operation.getDate()) << ","
                << operation.getCategoryId() << ","
                << operation.getDescription() << "\n";
    }
    
    std::string getResult() const override {
        return buffer_.str();
    }
    
    void reset() override {
        buffer_.str("");
        buffer_.clear();
        firstAccount_ = true;
        firstCategory_ = true;
        firstOperation_ = true;
    }
};

// JSON Export Visitor
class JSONExportVisitor : public IExportVisitor {
private:
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
           << "      \"isActive\": " << (account.getIsActive() ? "true" : "false") << "\n"
           << "    }";
        accounts_.push_back(ss.str());
    }
    
    void visit(const Category& category) override {
        std::stringstream ss;
        ss << "    {\n"
           << "      \"id\": \"" << category.getId() << "\",\n"
           << "      \"type\": \"" << categoryTypeToString(category.getType()) << "\",\n"
           << "      \"name\": \"" << category.getName() << "\",\n"
           << "      \"description\": \"" << category.getDescription() << "\"\n"
           << "    }";
        categories_.push_back(ss.str());
    }
    
    void visit(const Operation& operation) override {
        std::stringstream ss;
        ss << "    {\n"
           << "      \"id\": \"" << operation.getId() << "\",\n"
           << "      \"type\": \"" << operationTypeToString(operation.getType()) << "\",\n"
           << "      \"bankAccountId\": \"" << operation.getBankAccountId() << "\",\n"
           << "      \"amount\": " << operation.getAmount().getAmount() << ",\n"
           << "      \"currency\": \"" << operation.getAmount().getCurrency() << "\",\n"
           << "      \"date\": \"" << DateTimeUtils::toString(operation.getDate()) << "\",\n"
           << "      \"categoryId\": \"" << operation.getCategoryId() << "\",\n"
           << "      \"description\": \"" << operation.getDescription() << "\"\n"
           << "    }";
        operations_.push_back(ss.str());
    }
    
    std::string getResult() const override {
        std::stringstream result;
        result << "{\n";
        
        // Accounts
        result << "  \"accounts\": [\n";
        for (size_t i = 0; i < accounts_.size(); ++i) {
            result << accounts_[i];
            if (i < accounts_.size() - 1) result << ",";
            result << "\n";
        }
        result << "  ],\n";
        
        // Categories
        result << "  \"categories\": [\n";
        for (size_t i = 0; i < categories_.size(); ++i) {
            result << categories_[i];
            if (i < categories_.size() - 1) result << ",";
            result << "\n";
        }
        result << "  ],\n";
        
        // Operations
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

// Data Exporter using Visitor pattern
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
        
        // Visit all accounts
        for (const auto& account : accounts) {
            ExportableBankAccount exportable(*account);
            exportable.accept(*visitor_);
        }
        
        // Visit all categories
        for (const auto& category : categories) {
            ExportableCategory exportable(*category);
            exportable.accept(*visitor_);
        }
        
        // Visit all operations
        for (const auto& operation : operations) {
            ExportableOperation exportable(*operation);
            exportable.accept(*visitor_);
        }
        
        // Write to file
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw InfrastructureException("Cannot create file: " + filename);
        }
        
        file << visitor_->getResult();
        file.close();
    }
    
    std::string exportToString(
        const std::vector<std::shared_ptr<BankAccount>>& accounts,
        const std::vector<std::shared_ptr<Category>>& categories,
        const std::vector<std::shared_ptr<Operation>>& operations) {
        
        visitor_->reset();
        
        // Visit all entities
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

// Factory for creating exporters
class ExporterFactory {
public:
    static std::unique_ptr<DataExporter> create(const std::string& format) {
        if (format == "csv" || format == "CSV") {
            return std::make_unique<DataExporter>(std::make_unique<CSVExportVisitor>());
        } else if (format == "json" || format == "JSON") {
            return std::make_unique<DataExporter>(std::make_unique<JSONExportVisitor>());
        } else {
            throw std::invalid_argument("Unsupported export format: " + format);
        }
    }
};

} // namespace financial::infrastructure