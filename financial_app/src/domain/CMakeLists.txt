# Domain library
add_library(domain_lib INTERFACE)

target_include_directories(domain_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(domain_lib INTERFACE
        common_lib
)

# Add sources (header-only for now)
target_sources(domain_lib INTERFACE
        # Value Objects
        ${CMAKE_CURRENT_SOURCE_DIR}/value_objects/money.h
        ${CMAKE_CURRENT_SOURCE_DIR}/value_objects/types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/value_objects/date_range.h

        # Entities
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/category.h
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/bank_account.h
        ${CMAKE_CURRENT_SOURCE_DIR}/entities/operation.h

        # Factories
        ${CMAKE_CURRENT_SOURCE_DIR}/factories/entity_factory.h

        # Repositories
        ${CMAKE_CURRENT_SOURCE_DIR}/repositories/repository_interfaces.h

        # Services
        ${CMAKE_CURRENT_SOURCE_DIR}/services/domain_services.h
)