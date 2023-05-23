enable_testing()

include(GoogleTest)

add_executable(guiCoreTest
    widgetTest.cpp
    )

target_link_libraries(guiCoreTest
    ${LINK_UNIX_LIBRARIES}
    guiCore
    gtest_main)

gtest_discover_tests(guiCoreTest
    TEST_SUFFIX .guiCoreTest
)

add_test(NAME testGuiCore COMMAND guiCoreTest)
