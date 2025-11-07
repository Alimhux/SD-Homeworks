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

// Шаблон Builder для создания аналитических отчётов
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

        // Заголовок
        report_ << "========================================\n";
        report_ << title_ << "\n";
        report_ << "========================================\n\n";

        // Информация о периоде
        report_ << "Период: " << DateTimeUtils::toString(analytics_.period.getStart())
                << " - " << DateTimeUtils::toString(analytics_.period.getEnd()) << "\n\n";

        // Сводка
        if (includeSummary_) {
            buildSummary();
        }

        // Детали
        if (includeDetails_) {
            buildIncomeDetails();
            buildExpenseDetails();
        }

        // Графики (текстовые)
        if (includeCharts_) {
            buildCharts();
        }

        // Подвал
        report_ << "\n========================================\n";
        report_ << "Сгенерировано: " << DateTimeUtils::toString(DateTimeUtils::now()) << "\n";

        return report_.str();
    }

private:
    void buildSummary() {
        report_ << "СВОДКА\n";
        report_ << "-------\n";
        report_ << std::fixed << std::setprecision(2);
        report_ << "Общий доход:  " << std::setw(12) << analytics_.totalIncome.getAmount()
                << " " << analytics_.totalIncome.getCurrency() << "\n";
        report_ << "Общий расход: " << std::setw(12) << analytics_.totalExpense.getAmount()
                << " " << analytics_.totalExpense.getCurrency() << "\n";
        report_ << "Чистый доход: " << std::setw(12) << analytics_.netIncome.getAmount()
                << " " << analytics_.netIncome.getCurrency() << "\n\n";
    }

    void buildIncomeDetails() {
        if (analytics_.incomeByCategory.empty()) return;

        report_ << "ДОХОДЫ ПО КАТЕГОРИЯМ\n";
        report_ << "------------------\n";

        for (const auto& cat : analytics_.incomeByCategory) {
            report_ << std::left << std::setw(20) << cat.categoryName
                    << std::right << std::setw(12) << cat.totalAmount.getAmount()
                    << " (" << std::setw(5) << std::setprecision(1)
                    << cat.percentage << "%)"
                    << " [" << cat.operationCount << " операций]\n";
        }
        report_ << "\n";
    }

    void buildExpenseDetails() {
        if (analytics_.expenseByCategory.empty()) return;

        report_ << "РАСХОДЫ ПО КАТЕГОРИЯМ\n";
        report_ << "--------------------\n";

        for (const auto& cat : analytics_.expenseByCategory) {
            report_ << std::left << std::setw(20) << cat.categoryName
                    << std::right << std::setw(12) << cat.totalAmount.getAmount()
                    << " (" << std::setw(5) << std::setprecision(1)
                    << cat.percentage << "%)"
                    << " [" << cat.operationCount << " операций]\n";
        }
        report_ << "\n";
    }

    void buildCharts() {
        report_ << "РАСПРЕДЕЛЕНИЕ РАСХОДОВ (Текстовый график)\n";
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

// Фасад аналитики
class AnalyticsFacade {
private:
    std::shared_ptr<AnalyticsService> analyticsService_;
    std::shared_ptr<BalanceReconciliationService> reconciliationService_;
    std::shared_ptr<IOperationRepository> operationRepo_;
    std::shared_ptr<IBankAccountRepository> accountRepo_;
    std::shared_ptr<ICategoryRepository> categoryRepo_;

public:
    AnalyticsFacade() {
        analyticsService_ = ServiceLocator::get<AnalyticsService>();
        reconciliationService_ = ServiceLocator::get<BalanceReconciliationService>();
        operationRepo_ = ServiceLocator::get<IOperationRepository>();
        accountRepo_ = ServiceLocator::get<IBankAccountRepository>();
        categoryRepo_ = ServiceLocator::get<ICategoryRepository>();
    }

    // Аналитика по периоду
    PeriodAnalytics getAnalytics(const DateRange& period) {
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

    // Топ категорий
    std::vector<CategoryAnalytics> getTopIncomeCategories(size_t limit = 5) {
        return analyticsService_->getTopCategories(
            DateRange::thisMonth(), OperationType::INCOME, limit);
    }

    std::vector<CategoryAnalytics> getTopExpenseCategories(size_t limit = 5) {
        return analyticsService_->getTopCategories(
            DateRange::thisMonth(), OperationType::EXPENSE, limit);
    }

    // Согласование балансов
    AccountBalance checkBalance(const Id& accountId) {
        return reconciliationService_->checkAccountBalance(accountId);
    }

    std::vector<AccountBalance> checkAllBalances() {
        return reconciliationService_->checkAllBalances();
    }

    void recalculateBalance(const Id& accountId, bool autoFix = false) {
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

    // Генерация отчётов с использованием Builder
    std::string generateMonthlyReport() {
        auto analytics = getMonthAnalytics();

        return AnalyticsReportBuilder()
            .setTitle("Ежемесячный финансовый отчёт")
            .setAnalytics(analytics)
            .includeSummary(true)
            .includeDetails(true)
            .includeCharts(true)
            .build();
    }

    std::string generateYearlyReport() {
        auto analytics = getYearAnalytics();

        return AnalyticsReportBuilder()
            .setTitle("Годовой финансовый отчёт")
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

    // Экспорт данных
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

    void importFromJSON(const std::string& filename) {
        auto importer = ImporterFactory::create("json");
        auto data = importer->import(filename);

        // Обработка импортированных данных
        processImportedData(data);
    }

    // Статистика
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

        // Импорт счетов
        for (const auto& accountDTO : data.accounts) {
            auto account = factory->createBankAccount(
                accountDTO.name,
                Money(accountDTO.balance, accountDTO.currency),
                accountDTO.accountNumber
            );
            accountRepo_->save(account);
        }

        // Импорт категорий
        for (const auto& categoryDTO : data.categories) {
            auto type = stringToCategoryType(categoryDTO.type);
            auto category = factory->createCategory(
                type,
                categoryDTO.name,
                categoryDTO.description
            );
            categoryRepo_->save(category);
        }

        // Импорт операций
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