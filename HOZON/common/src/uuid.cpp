#include "uuid.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <regex>

namespace hozon {

std::string UUID::generate() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // Формат: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    ss << std::setw(8) << ((part1 >> 32) & 0xFFFFFFFF) << "-";
    ss << std::setw(4) << ((part1 >> 16) & 0xFFFF) << "-";
    ss << std::setw(4) << (((part1 & 0xFFFF) & 0x0FFF) | 0x4000) << "-";
    ss << std::setw(4) << (((part2 >> 48) & 0x3FFF) | 0x8000) << "-";
    ss << std::setw(12) << (part2 & 0xFFFFFFFFFFFF);

    return ss.str();
}

bool UUID::isValid(const std::string& uuid) {
    static const std::regex uuidRegex(
        "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$",
        std::regex::icase
    );
    return std::regex_match(uuid, uuidRegex);
}

} // namespace hozon
