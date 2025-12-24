cmake_minimum_required(VERSION 3.16)
project(api-gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common library sources (built inline for standalone build)
set(COMMON_SOURCES
    ../common/src/message_types.cpp
    ../common/src/http_server.cpp
    ../common/src/http_client.cpp
    ../common/src/database.cpp
    ../common/src/rabbitmq.cpp
    ../common/src/uuid.cpp
)

set(SOURCES
    src/main.cpp
    src/router.cpp
)

add_executable(api-gateway ${SOURCES} ${COMMON_SOURCES})

target_include_directories(api-gateway PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
)

find_package(Threads REQUIRED)
target_link_libraries(api-gateway PRIVATE
    Threads::Threads
    pqxx
    pq
    rabbitmq
)
