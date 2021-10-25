add_executable(bakTest
    characterTest.cpp
    inventoryTest.cpp
    partyTest.cpp
    )

target_link_libraries(bakTest
    stdc++
    m
    bak
    com
    gtest_main)

add_custom_target(testBak ALL COMMAND bakTest)
