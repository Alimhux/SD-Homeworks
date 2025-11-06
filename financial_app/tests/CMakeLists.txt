# Tests
enable_testing()

# Add test executable
#add_executable(financial_tests
#        # Test files will be added here
#        # test_main.cpp
#        # domain_tests.cpp
#        # application_tests.cpp
#        # infrastructure_tests.cpp
#)

#target_link_libraries(financial_tests
#        domain_lib
#        application_lib
#        infrastructure_lib
#        presentation_lib
#        common_lib
#        Threads::Threads
#)

# Add tests
add_test(NAME financial_tests COMMAND financial_tests)