#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include "domain/services/domain_services.h"
#include "infrastructure/serialization/data_exporter.h"
#include "infrastructure/serialization/data_importer.h"

namespace financial::application {

using namespace financial::domain;
using namespace financial::infrastructure;

// Report Builder pattern for creating analytics reports
class AnalyticsReportBuilder {
private:
    std::stringstream report_;
    PeriodAnalytics analytics_;
    std::string title_;
    bool includeCharts_ = false;
    bool includeDetails_ = true;
    bool includeSummary_ = true;

public:
    AnalyticsReportBuilder& setTitle(const std::string& title) {
        title_ = title;
        return *this;
    }

    AnalyticsReportBuilder& setAnalytics(const PeriodAnalytics& analytics) {
        analytics_ = analytics;
        return *this;
    }

    AnalyticsReportBuilder& includeCharts(bool include = true) {
        includeCharts_ = include;
        return *this;
    }

    AnalyticsReportBuilder& includeDetails(bool include = true) {
        includeDetails_ = include;
        return *this;
    }

    AnalyticsReportBuilder& includeSummary(bool include = true) {
        includeSummary_ = include;
        return *this;
    }

    std::string build() {
        report_.str("");
        report_.clear();

        // Header
        report_ << "========================================\n";
        report_ << title_ << "\n";
        report_ << "========================================\n\n";

        // Period info
        report_ << "Period: " << DateTimeUtils::toString(analytics_.period.getStart())
                << " - " << DateTimeUtils::toString(analytics_.period.getEnd()) << "\n\n";

        // Summary
        if (includeSummary_) {
            buildSummary();
        }

        // Details
        if (includeDetails_) {
            buildIncomeDetails();
            buildExpenseDetails();
        }

        // Charts (text-based)
        if (includeCharts_) {
            buildCharts();
        }

        // Footer
        report_ << "\n========================================\n";
        report_ << "Generated: " << DateTimeUtils::toString(DateTimeUtils::now()) << "\n";

        return report_.str();
    }

private:
    void buildSummary() {
        report_ << "SUMMARY\n";
        report_ << "-------\n";
        report_ << std::fixed << std::setprecision(2);
        report_ << "Total Income:  " << std::setw(12) << analytics_.totalIncome.getAmount()
                << " " << analytics_.totalIncome.getCurrency() << "\n";
        report_ << "Total Expense: " << std::setw(12) << analytics_.totalExpense.getAmount()
                << " " << analytics_.totalExpense.getCurrency() << "\n";
        report_ << "Net Income:    " << std::setw(12) << analytics_.netIncome.getAmount()
                << " " << analytics_.netIncome.getCurrency() << "\n\n";
    }

    void buildIncomeDetails() {
        if (analytics_.incomeByCategory.empty()) return;

        report_ << "INCOME BY CATEGORY\n";
        report_ << "------------------\n";

        for (const auto& cat : analytics_.incomeByCategory) {
            report_ << std::left << std::setw(20) << cat.categoryName
                    << std::right << std::setw(12) << cat.totalAmount.getAmount()
                    << " (" << std::setw(5) << std::setprecision(1)
                    << cat.percentage << "%)"
                    << " [" << cat.operationCount << " operations]\n";
        }
        report_ << "\n";
    }

    void buildExpenseDetails() {
        if (analytics_.expenseByCategory.empty()) return;

        report_ << "EXPENSES BY CATEGORY\n";
        report_ << "--------------------\n";

        for (const auto& cat : analytics_.expenseByCategory) {
            report_ << std::left << std::setw(20) << cat.categoryName
                    << std::right << std::setw(12) << cat.totalAmount.getAmount()
                    << " (" << std::setw(5) << std::setprecision(1)
                    << cat.percentage << "%)"
                    << " [" << cat.operationCount << " operations]\n";
        }
        report_ << "\n";
    }

    void buildCharts() {
        report_ << "EXPENSE DISTRIBUTION (Text Chart)\n";
        report_ << "---------------------------------\n";

        const int maxWidth = 40;
        for (const auto& cat : analytics_.expenseByCategory) {
            int barLength = static_cast<int>((cat.percentage / 100.0) * maxWidth);
            report_ << std::left << std::setw(15) << cat.categoryName << " |";

            for (int i = 0; i < barLength; ++i) {
                report_ << "█";
            }
            report_ << " " << std::setprecision(1) << cat.percentage << "%\n";
        }
        report_ << "\n";
    }
};

// Analytics Facade
class AnalyticsFacade {
private:
    std::shared_ptr<AnalyticsService> analyticsService_;
    std::shared_ptr<BalanceReconciliationService> reconciliationService_;
    std::shared_ptr<IOperationRepository> operationRepo_;
    std::shared_ptr<IBankAccountRepository> accountRepo_;
    std::shared_ptr<ICategoryRepository> categoryRepo_;

public:
    // Небольшое пояснение: в этом классе, для избежания
    // цилической зависимости мне пришлось сделать
    // ленивую инициализацию первых двух полей. Другого
    // решения я к сожалению не нашёл
    AnalyticsFacade() {
        // analyticsService_ = ServiceLocator::get<AnalyticsService>();
        // reconciliationService_ = ServiceLocator::get<BalanceReconciliationService>();
        operationRepo_ = ServiceLocator::get<IOperationRepository>();
        accountRepo_ = ServiceLocator::get<IBankAccountRepository>();
        categoryRepo_ = ServiceLocator::get<ICategoryRepository>();
    }

    // Period analytics
    PeriodAnalytics getAnalytics(const DateRange& period) {
        if (!analyticsService_) {
          analyticsService_ = ServiceLocator::get<AnalyticsService>();

        }
        return analyticsService_->calculatePeriodAnalytics(period);
    }

    PeriodAnalytics getTodayAnalytics() {
        return getAnalytics(DateRange::today());
    }

    PeriodAnalytics getMonthAnalytics() {
        return getAnalytics(DateRange::thisMonth());
    }

    PeriodAnalytics getYearAnalytics() {
        return getAnalytics(DateRange::thisYear());
    }

    PeriodAnalytics getCustomPeriodAnalytics(const DateTime& start, const DateTime& end) {
        return getAnalytics(DateRange(start, end));
    }

    // Top categories
    std::vector<CategoryAnalytics> getTopIncomeCategories(size_t limit = 5) {
      if (!analyticsService_) {
        analyticsService_ = ServiceLocator::get<AnalyticsService>();
      }
        return analyticsService_->getTopCategories(
            DateRange::thisMonth(), OperationType::INCOME, limit);
    }

    std::vector<CategoryAnalytics> getTopExpenseCategories(size_t limit = 5) {
      if (!analyticsService_) {
        analyticsService_ = ServiceLocator::get<AnalyticsService>();
      }
        return analyticsService_->getTopCategories(
            DateRange::thisMonth(), OperationType::EXPENSE, limit);
    }

    // Balance reconciliation
    AccountBalance checkBalance(const Id& accountId) {
      if (reconciliationService_) {
        reconciliationService_ = ServiceLocator::get<BalanceReconciliationService>();
      }
        return reconciliationService_->checkAccountBalance(accountId);
    }

    std::vector<AccountBalance> checkAllBalances() {
      if (reconciliationService_) {
        reconciliationService_ = ServiceLocator::get<BalanceReconciliationService>();
      }
        return reconciliationService_->checkAllBalances();
    }

    void recalculateBalance(const Id& accountId, bool autoFix = false) {
      if (reconciliationService_) {
        reconciliationService_ = ServiceLocator::get<BalanceReconciliationService>();
      }
        reconciliationService_->recalculateBalance(accountId, autoFix);
    }

    void fixAllBalanceDiscrepancies() {
        auto balances = checkAllBalances();
        for (const auto& balance : balances) {
            if (balance.hasDiscrepancy) {
                recalculateBalance(balance.accountId, true);
            }
        }
    }

    // Report generation using Builder
    std::string generateMonthlyReport() {
      std::cout << "OK" << '\n';

        auto analytics = getMonthAnalytics();
        std::cout << "OK" << '\n';
        return AnalyticsReportBuilder()
            .setTitle("Monthly Financial Report")
            .setAnalytics(analytics)
            .includeSummary(true)
            .includeDetails(true)
            .includeCharts(true)
            .build();
    }

    std::string generateYearlyReport() {
        auto analytics = getYearAnalytics();

        return AnalyticsReportBuilder()
            .setTitle("Yearly Financial Report")
            .setAnalytics(analytics)
            .includeSummary(true)
            .includeDetails(true)
            .includeCharts(false)
            .build();
    }

    std::string generateCustomReport(const DateRange& period, const std::string& title) {
        auto analytics = getAnalytics(period);

        return AnalyticsReportBuilder()
            .setTitle(title)
            .setAnalytics(analytics)
            .includeSummary(true)
            .includeDetails(true)
            .includeCharts(true)
            .build();
    }

    // Data export
    void exportToCSV(const std::string& filename) {
        auto exporter = ExporterFactory::create("csv");
        exporter->exportToFile(
            filename,
            accountRepo_->findAll(),
            categoryRepo_->findAll(),
            operationRepo_->findAll()
        );
    }

    void exportToJSON(const std::string& filename) {
        auto exporter = ExporterFactory::create("json");
        exporter->exportToFile(
            filename,
            accountRepo_->findAll(),
            categoryRepo_->findAll(),
            operationRepo_->findAll()
        );
    }

    // Data import
    void importFromCSV(const std::string& filename) {
        auto importer = ImporterFactory::create("csv");
        auto data = importer->import(filename);

        // Process imported data
        processImportedData(data);
    }

    void importFromJSON(const std::string& filename) {
        auto importer = ImporterFactory::create("json");
        auto data = importer->import(filename);

        // Process imported data
        processImportedData(data);
    }

    // Statistics
    Money calculateAverageMonthlyIncome() {
        auto yearAnalytics = getYearAnalytics();
        if (yearAnalytics.totalIncome.isZero()) {
            return Money::zero();
        }
        return yearAnalytics.totalIncome.multiply(1.0 / 12.0);
    }

    Money calculateAverageMonthlyExpense() {
        auto yearAnalytics = getYearAnalytics();
        if (yearAnalytics.totalExpense.isZero()) {
            return Money::zero();
        }
        return yearAnalytics.totalExpense.multiply(1.0 / 12.0);
    }

    double calculateSavingsRate() {
        auto monthAnalytics = getMonthAnalytics();
        if (monthAnalytics.totalIncome.isZero()) {
            return 0.0;
        }

        double savingsRate = (monthAnalytics.netIncome.getAmount() /
                             monthAnalytics.totalIncome.getAmount()) * 100;
        return savingsRate;
    }

private:
    void processImportedData(const ImportData& data) {
        auto factory = ServiceLocator::get<IEntityFactory>();

        // Import accounts
        for (const auto& accountDTO : data.accounts) {
            auto account = factory->createBankAccount(
                accountDTO.name,
                Money(accountDTO.balance, accountDTO.currency),
                accountDTO.accountNumber
            );
            accountRepo_->save(account);
        }

        // Import categories
        for (const auto& categoryDTO : data.categories) {
            auto type = stringToCategoryType(categoryDTO.type);
            auto category = factory->createCategory(
                type,
                categoryDTO.name,
                categoryDTO.description
            );
            categoryRepo_->save(category);
        }

        // Import operations
        for (const auto& operationDTO : data.operations) {
            auto type = stringToOperationType(operationDTO.type);
            auto operation = factory->createOperation(
                type,
                operationDTO.bankAccountId,
                Money(operationDTO.amount, operationDTO.currency),
                operationDTO.categoryId,
                operationDTO.description,
                DateTimeUtils::fromString(operationDTO.date)
            );
            operationRepo_->save(operation);
        }
    }
};

} // namespace financial::application