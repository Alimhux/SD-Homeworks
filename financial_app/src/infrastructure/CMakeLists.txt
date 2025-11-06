# Infrastructure library
add_library(infrastructure_lib INTERFACE)

target_include_directories(infrastructure_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(infrastructure_lib INTERFACE
        domain_lib
        common_lib
)

# Add sources (header-only for now)
target_sources(infrastructure_lib INTERFACE
        # Persistence
        ${CMAKE_CURRENT_SOURCE_DIR}/persistence/in_memory_repository.h

        # Proxy
        ${CMAKE_CURRENT_SOURCE_DIR}/proxy/caching_proxy.h

        # DI Container
        ${CMAKE_CURRENT_SOURCE_DIR}/di/di_container.h

        # Serialization
        ${CMAKE_CURRENT_SOURCE_DIR}/serialization/data_importer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialization/data_exporter.h
)