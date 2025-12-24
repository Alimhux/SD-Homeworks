cmake_minimum_required(VERSION 3.16)
project(common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common library sources
set(COMMON_SOURCES
    src/message_types.cpp
    src/http_server.cpp
    src/http_client.cpp
    src/database.cpp
    src/rabbitmq.cpp
    src/uuid.cpp
)

set(COMMON_HEADERS
    include/json.h
    include/message_types.h
    include/http_server.h
    include/http_client.h
    include/database.h
    include/rabbitmq.h
    include/uuid.h
    include/config.h
)

add_library(common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(common PUBLIC
    Threads::Threads
    pqxx
    pq
    rabbitmq
)
