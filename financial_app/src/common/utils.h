#pragma once

#include <string>
#include <random>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "types.h"

namespace financial {

class IdGenerator {
private:
    static std::random_device rd_;
    static std::mt19937 gen_;
    static std::uniform_int_distribution<> dis_;

public:
    static std::string generate(const std::string& prefix = "") {
        std::stringstream ss;
        if (!prefix.empty()) {
            ss << prefix << "-";
        }

        // Generate timestamp part
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        ss << std::hex << time_t;

        // Add random part
        ss << "-";
        for (int i = 0; i < 8; ++i) {
            ss << std::hex << dis_(gen_);
        }

        return ss.str();
    }
};

// Static member definitions
inline std::random_device IdGenerator::rd_;
inline std::mt19937 IdGenerator::gen_(rd_());
inline std::uniform_int_distribution<> IdGenerator::dis_(0, 15);

class DateTimeUtils {
public:
    static DateTime now() {
        return std::chrono::system_clock::now();
    }

    static std::string toString(const DateTime& dt) {
        auto time_t = std::chrono::system_clock::to_time_t(dt);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static DateTime fromString(const std::string& str) {
        std::tm tm = {};
        std::stringstream ss(str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }

    static DateTime startOfDay(const DateTime& dt) {
        auto time_t = std::chrono::system_clock::to_time_t(dt);
        std::tm* tm = std::localtime(&time_t);
        tm->tm_hour = 0;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        return std::chrono::system_clock::from_time_t(std::mktime(tm));
    }

    static DateTime endOfDay(const DateTime& dt) {
        auto time_t = std::chrono::system_clock::to_time_t(dt);
        std::tm* tm = std::localtime(&time_t);
        tm->tm_hour = 23;
        tm->tm_min = 59;
        tm->tm_sec = 59;
        return std::chrono::system_clock::from_time_t(std::mktime(tm));
    }
};

// Performance measurement utility for decorator pattern
class PerformanceTimer {
private:
    std::chrono::steady_clock::time_point start_;
    std::string operation_;

public:
    explicit PerformanceTimer(const std::string& operation)
        : start_(std::chrono::steady_clock::now()), operation_(operation) {}

    ~PerformanceTimer() {}

    long long elapsed() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
    }
};

} // namespace financial