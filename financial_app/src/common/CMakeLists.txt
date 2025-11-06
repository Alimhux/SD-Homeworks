# Common library
add_library(common_lib INTERFACE)

target_include_directories(common_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add sources (header-only for now)
target_sources(common_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/exceptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/validation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
)