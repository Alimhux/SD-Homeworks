# Application library
add_library(application_lib INTERFACE)

target_include_directories(application_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(application_lib INTERFACE
        domain_lib
        infrastructure_lib
        common_lib
)

# Add sources (header-only for now)
target_sources(application_lib INTERFACE
        # Commands
        ${CMAKE_CURRENT_SOURCE_DIR}/commands/commands.h

        # Decorators
        ${CMAKE_CURRENT_SOURCE_DIR}/decorators/command_decorators.h

        # Facades
        ${CMAKE_CURRENT_SOURCE_DIR}/facades/account_facade.h
        ${CMAKE_CURRENT_SOURCE_DIR}/facades/operation_facade.h
        ${CMAKE_CURRENT_SOURCE_DIR}/facades/analytics_facade.h
)