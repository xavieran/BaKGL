enable_testing()

include(GoogleTest)

add_executable(bakTest
    timeTest.cpp
    characterTest.cpp
    keyContainerTest.cpp
    lockTest.cpp
    inventoryTest.cpp
    partyTest.cpp
    skillTest.cpp
    templeTest.cpp
    )

target_link_libraries(bakTest
    ${LINK_UNIX_LIBRARIES}
    bak
    com
    gtest_main)

gtest_discover_tests(bakTest
    TEST_SUFFIX .bakTest
)

add_test(NAME testBak COMMAND bakTest)
