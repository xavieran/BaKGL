enable_testing()

include(GoogleTest)

add_executable(combatTest
    calculationsTest.cpp
    )

target_link_libraries(combatTest
    ${LINK_UNIX_LIBRARIES}
    bak
    com
    gtest_main)

gtest_discover_tests(combatTest
    TEST_SUFFIX .combatTest
)

add_test(NAME testCombat COMMAND combatTest)
