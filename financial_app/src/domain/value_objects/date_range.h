#pragma once

#include "common/types.h"
#include "common/utils.h"
#include "common/validation.h"

namespace financial::domain {

// DateRange класс для фильтрации операций по дате
class DateRange : public ValueObject {
 private:
  DateTime start_;
  DateTime end_;

 public:
  DateRange() = default;

  DateRange(const DateTime& start, const DateTime& end)
      : start_(start), end_(end) {
    if (start > end) {
      throw ValidationException("Start date must be before end date");
    }
  }

  const DateTime& getStart() const { return start_; }
  const DateTime& getEnd() const { return end_; }

  bool contains(const DateTime& date) const {
    return date >= start_ && date <= end_;
  }

  bool overlaps(const DateRange& other) const {
    return start_ <= other.end_ && end_ >= other.start_;
  }

  bool equals(const ValueObject& other) const override {
    auto* otherRange = dynamic_cast<const DateRange*>(&other);
    if (!otherRange) return false;
    return start_ == otherRange->start_ && end_ == otherRange->end_;
  }

  static DateRange today() {
    auto now = DateTimeUtils::now();
    return DateRange(DateTimeUtils::startOfDay(now),
                     DateTimeUtils::endOfDay(now));
  }

  static DateRange thisMonth() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);

    // Начало месяца
    tm->tm_mday = 1;
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    auto start = std::chrono::system_clock::from_time_t(std::mktime(tm));

    // Конец месяца
    tm->tm_mon += 1;
    tm->tm_mday = 0;
    tm->tm_hour = 23;
    tm->tm_min = 59;
    tm->tm_sec = 59;
    auto end = std::chrono::system_clock::from_time_t(std::mktime(tm));

    return DateRange(start, end);
  }

  static DateRange thisYear() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);

    // Начало года
    tm->tm_mon = 0;
    tm->tm_mday = 1;
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    auto start = std::chrono::system_clock::from_time_t(std::mktime(tm));

    // Конец года
    tm->tm_mon = 11;
    tm->tm_mday = 31;
    tm->tm_hour = 23;
    tm->tm_min = 59;
    tm->tm_sec = 59;
    auto end = std::chrono::system_clock::from_time_t(std::mktime(tm));

    return DateRange(start, end);
  }
};

}  // namespace financial::domain