enable_testing()

include(GoogleTest)

add_executable(bakTest
    characterTest.cpp
    keyContainerTest.cpp
    lockTest.cpp
    inventoryTest.cpp
    partyTest.cpp
    skillTest.cpp
    )

target_link_libraries(bakTest
    bak
    com
    gtest_main)

gtest_discover_tests(bakTest
    TEST_SUFFIX .bakTest
)

add_test(NAME testBak COMMAND bakTest)
