enable_testing()

include(GoogleTest)

add_executable(guiTest
    widgetTest.cpp
    )

target_link_libraries(guiTest
    #stdc++
    #m
    gui
    gtest_main)

gtest_discover_tests(guiTest
    TEST_SUFFIX .guiTest
)

add_test(NAME testGui COMMAND guiTest)
