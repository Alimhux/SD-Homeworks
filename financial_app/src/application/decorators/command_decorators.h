#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <map>
#include <sstream>
#include <climits>
#include "application/commands/commands.h"
#include "common/utils.h"
#include "infrastructure/di/di_container.h"

namespace financial::application {

// Базовый декоратор для команд
class CommandDecorator : public ICommand {
protected:
    std::shared_ptr<ICommand> wrappedCommand_;

public:
    explicit CommandDecorator(std::shared_ptr<ICommand> command)
        : wrappedCommand_(command) {}

    std::string getName() const override {
        return wrappedCommand_->getName();
    }

    bool canUndo() const override {
        return wrappedCommand_->canUndo();
    }
};

// Декоратор измерения производительности
class PerformanceMeasuringDecorator : public CommandDecorator {
private:
    mutable long long executionTime_ = 0;
    mutable long long undoTime_ = 0;

public:
    explicit PerformanceMeasuringDecorator(std::shared_ptr<ICommand> command)
        : CommandDecorator(command) {}

    void execute() override {
        auto start = std::chrono::high_resolution_clock::now();

        wrappedCommand_->execute();

        auto end = std::chrono::high_resolution_clock::now();
        executionTime_ = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    void undo() override {
        auto start = std::chrono::high_resolution_clock::now();

        wrappedCommand_->undo();

        auto end = std::chrono::high_resolution_clock::now();
        undoTime_ = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    long long getExecutionTime() const { return executionTime_; }
    long long getUndoTime() const { return undoTime_; }

    std::string getPerformanceReport() const {
        std::stringstream ss;
        ss << "Команда: " << getName() << "\n"
           << "  Время выполнения: " << executionTime_ << " мкс\n"
           << "  Время отмены: " << undoTime_ << " мкс\n";
        return ss.str();
    }
};

// Декоратор логирования
class LoggingDecorator : public CommandDecorator {
private:
    static std::mutex logMutex_;
    static std::ofstream logFile_;

    static void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex_);

        auto now = DateTimeUtils::now();
        std::string timestamp = DateTimeUtils::toString(now);

        // Лог в консоль
        std::cout << "[" << timestamp << "] " << message << std::endl;

        if (logFile_.is_open()) {
            logFile_ << "[" << timestamp << "] " << message << std::endl;
            logFile_.flush();
        }
    }

public:
    explicit LoggingDecorator(std::shared_ptr<ICommand> command)
        : CommandDecorator(command) {}

    static void openLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(logMutex_);
        if (logFile_.is_open()) {
            logFile_.close();
        }
        logFile_.open(filename, std::ios::app);
    }

    static void closeLogFile() {
        std::lock_guard<std::mutex> lock(logMutex_);
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    void execute() override {
        log("Выполнение команды: " + getName());
        try {
            wrappedCommand_->execute();
            log("Успешно выполнено: " + getName());
        } catch (const std::exception& e) {
            log("Ошибка при выполнении " + getName() + ": " + e.what());
            throw;
        }
    }

    void undo() override {
        log("Отмена команды: " + getName());
        try {
            wrappedCommand_->undo();
            log("Успешно отменено: " + getName());
        } catch (const std::exception& e) {
            log("Ошибка при отмене " + getName() + ": " + e.what());
            throw;
        }
    }
};

inline std::mutex LoggingDecorator::logMutex_;
inline std::ofstream LoggingDecorator::logFile_;

// Декоратор валидации
class ValidationDecorator : public CommandDecorator {
public:
    explicit ValidationDecorator(std::shared_ptr<ICommand> command)
        : CommandDecorator(command) {}

    void execute() override {
        validateBeforeExecution();
        wrappedCommand_->execute();
        validateAfterExecution();
    }

    void undo() override {
        if (!canUndo()) {
            throw std::runtime_error("Команда не может быть отменена: " + getName());
        }
        wrappedCommand_->undo();
    }

protected:
    virtual void validateBeforeExecution() {}

    virtual void validateAfterExecution() {}
};

// Декоратор транзакции (обеспечивает атомарность)
class TransactionDecorator : public CommandDecorator {
private:
    std::shared_ptr<domain::IUnitOfWork> unitOfWork_;

public:
    TransactionDecorator(std::shared_ptr<ICommand> command,
                         std::shared_ptr<domain::IUnitOfWork> unitOfWork)
        : CommandDecorator(command), unitOfWork_(unitOfWork) {}

    void execute() override {
        unitOfWork_->begin();
        try {
            wrappedCommand_->execute();
            unitOfWork_->commit();
        } catch (...) {
            unitOfWork_->rollback();
            throw;
        }
    }

    void undo() override {
        unitOfWork_->begin();
        try {
            wrappedCommand_->undo();
            unitOfWork_->commit();
        } catch (...) {
            unitOfWork_->rollback();
            throw;
        }
    }
};

// Сборщик статистики производительности
class PerformanceStatistics {
private:
    static std::unique_ptr<PerformanceStatistics> instance_;
    static std::mutex mutex_;

    struct Statistics {
        size_t count = 0;
        long long totalTime = 0;
        long long minTime = LLONG_MAX;
        long long maxTime = 0;

        void update(long long time) {
            count++;
            totalTime += time;
            minTime = std::min(minTime, time);
            maxTime = std::max(maxTime, time);
        }

        double average() const {
            return count > 0 ? static_cast<double>(totalTime) / count : 0;
        }
    };

    std::map<std::string, Statistics> commandStats_;
    mutable std::mutex statsMutex_;

    PerformanceStatistics() = default;

public:
    static PerformanceStatistics& getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_ = std::unique_ptr<PerformanceStatistics>(new PerformanceStatistics());
        }
        return *instance_;
    }

    void recordExecution(const std::string& commandName, long long time) {
        std::lock_guard<std::mutex> lock(statsMutex_);
        commandStats_[commandName].update(time);
    }

    std::string generateReport() const {
        std::lock_guard<std::mutex> lock(statsMutex_);
        std::stringstream ss;

        for (const auto& [name, stats] : commandStats_) {
            ss << "Команда: " << name << "\n";
            ss << "  Выполнений: " << stats.count << "\n";
            ss << "  Среднее время: " << stats.average() << " мкс\n";
            ss << "  Мин. время: " << stats.minTime << " мкс\n";
            ss << "  Макс. время: " << stats.maxTime << " мкс\n";
            ss << "  Общее время: " << stats.totalTime << " мкс\n\n";
        }

        return ss.str();
    }

    void reset() {
        std::lock_guard<std::mutex> lock(statsMutex_);
        commandStats_.clear();
    }
};

inline std::unique_ptr<PerformanceStatistics> PerformanceStatistics::instance_ = nullptr;
inline std::mutex PerformanceStatistics::mutex_;

// Фабрика для создания декорированных команд
class DecoratedCommandFactory {
public:
    enum DecorationType {
        NONE = 0,
        PERFORMANCE = 1 << 0,
        LOGGING = 1 << 1,
        VALIDATION = 1 << 2,
        TRANSACTION = 1 << 3,
        ALL = PERFORMANCE | LOGGING | VALIDATION | TRANSACTION
    };

    static std::shared_ptr<ICommand> decorate(
        std::shared_ptr<ICommand> command,
        int decorationFlags = PERFORMANCE | LOGGING) {

        if (decorationFlags & TRANSACTION) {
            auto unitOfWork = ServiceLocator::get<domain::IUnitOfWork>();
            command = std::make_shared<TransactionDecorator>(command, unitOfWork);
        }

        if (decorationFlags & VALIDATION) {
            command = std::make_shared<ValidationDecorator>(command);
        }

        if (decorationFlags & PERFORMANCE) {
            auto perfCommand = std::make_shared<PerformanceMeasuringDecorator>(command);
            // Подключение к сборщику статистики
            command = perfCommand;
        }

        if (decorationFlags & LOGGING) {
            command = std::make_shared<LoggingDecorator>(command);
        }

        return command;
    }
};

} // namespace financial::application