# Presentation library
add_library(presentation_lib INTERFACE)

target_include_directories(presentation_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(presentation_lib INTERFACE
        application_lib
        domain_lib
        common_lib
)

# Add sources (header-only for now)
target_sources(presentation_lib INTERFACE
        # Console UI
        ${CMAKE_CURRENT_SOURCE_DIR}/console/console_ui.h
)