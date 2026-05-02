add_executable(AppLogicTests src/AppLogicTests.cpp)
target_link_libraries(AppLogicTests
    gtest_main
    client_app_lib
    client_ui_lib
    client_fix_lib
    client_orderbook_lib
    client_admin_lib
)
gtest_discover_tests(AppLogicTests)
