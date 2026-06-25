enable_testing()

include(GoogleTest)

add_executable(combatGridTest
    gridAlgorithmsTest.cpp
    )

target_link_libraries(combatGridTest
    ${LINK_UNIX_LIBRARIES}
    game
    gtest_main)

gtest_discover_tests(combatGridTest
    TEST_SUFFIX .combatGridTest
)

add_test(NAME testCombatGrid COMMAND combatGridTest)
