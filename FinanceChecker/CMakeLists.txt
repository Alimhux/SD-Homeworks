cmake_minimum_required(VERSION 3.14)
project(FinanceTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем директории с заголовочными файлами
include_directories(include)

# ============= Подключение библиотек =============

# nlohmann/json для работы с JSON
include(FetchContent)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# yaml-cpp для работы с YAML
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG yaml-cpp-0.7.0
)
FetchContent_MakeAvailable(yaml-cpp)


add_executable(FinanceTracker
        src/main.cpp
        src/domain/BankAccount.cpp
        src/domain/Category.cpp
        src/domain/Operation.cpp
)

# Линкуем библиотеки
target_link_libraries(FinanceTracker
        nlohmann_json::nlohmann_json
        yaml-cpp
)