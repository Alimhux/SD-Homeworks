cmake_minimum_required(VERSION 3.14)
project(ZooERP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# ============= Google Test =============
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)

# Для Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ============= Test executable =============
# Main executable
add_executable(ZooERP
        src/main.cpp
        src/domain/Animal.cpp
        src/domain/Herbivore.cpp
        src/domain/Thing.cpp
        src/services/VeterinaryClinic.cpp
        src/services/ZooRepository.cpp
        src/services/ZooManager.cpp
        src/ui/ConsoleUI.cpp
)

# Tests
add_executable(ZooTests
        tests/TestMain.cpp
        tests/DIContainerTest.cpp
        tests/AnimalTest.cpp
        tests/HerbivoreTest.cpp
        tests/PredatorTest.cpp
        tests/ThingTest.cpp
        tests/VeterinaryClinicTest.cpp
        tests/ZooRepositoryTest.cpp
        tests/ZooManagerTest.cpp
        tests/DIConfigTest.cpp
        tests/ConsoleUITest.cpp
        src/domain/Animal.cpp
        src/domain/Herbivore.cpp
        src/domain/Thing.cpp
        src/services/VeterinaryClinic.cpp
        src/services/ZooRepository.cpp
        src/services/ZooManager.cpp
        src/ui/ConsoleUI.cpp
)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "include/*.h")

#add_executable(ZooERP ${SOURCE_FILES})
target_sources(ZooERP PUBLIC ${HEADER_FILES})

#target_sources(ZooTests PRIVATE ${SOURCE_FILES})

target_link_libraries(ZooTests
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(ZooTests)