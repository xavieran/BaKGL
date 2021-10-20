add_executable(guiTest
    widgetTest.cpp
    )

target_link_libraries(guiTest gui gtest_main)

add_custom_target(testGui ALL COMMAND guiTest)
