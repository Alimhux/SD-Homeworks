cmake_minimum_required(VERSION 3.16)
project(orders-service LANGUAGES CXX)

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
    src/handlers/order_handler.cpp
    src/repository/order_repository.cpp
    src/workers/outbox_worker.cpp
    src/workers/result_consumer.cpp
)

add_executable(orders-service ${SOURCES} ${COMMON_SOURCES})

target_include_directories(orders-service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
)

find_package(Threads REQUIRED)
target_link_libraries(orders-service PRIVATE
    Threads::Threads
    pqxx
    pq
    rabbitmq
)
