#ifndef HOZON_UUID_H
#define HOZON_UUID_H

#include <string>

namespace hozon {

class UUID {
public:
    // Generate a new random UUID
    static std::string generate();

    // Validate UUID format
    static bool isValid(const std::string& uuid);
};

} // namespace hozon

#endif // HOZON_UUID_H
