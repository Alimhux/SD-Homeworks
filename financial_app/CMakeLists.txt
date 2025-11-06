cmake_minimum_required(VERSION 3.16)
project(FinancialAccounting VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find packages
find_package(Threads REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Main executable
add_executable(financial_app
        src/main.cpp
)

#target_link_libraries(financial_app
#        domain_lib
#        application_lib
#        infrastructure_lib
#        presentation_lib
#        common_lib
#        Threads::Threads
#)

# Enable testing
enable_testing()
add_subdirectory(tests EXCLUDE_FROM_ALL)