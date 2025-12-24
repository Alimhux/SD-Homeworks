cmake_minimum_required(VERSION 3.16)
project(payments-service LANGUAGES CXX)

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
    src/handlers/payment_handler.cpp
    src/repository/account_repository.cpp
    src/workers/inbox_worker.cpp
    src/workers/outbox_worker.cpp
)

add_executable(payments-service ${SOURCES} ${COMMON_SOURCES})

target_include_directories(payments-service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
)

find_package(Threads REQUIRED)
target_link_libraries(payments-service PRIVATE
    Threads::Threads
    pqxx
    pq
    rabbitmq
)
