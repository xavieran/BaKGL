# Orderbook Tests

add_executable(bakTest
    inventoryTest.cpp
    partyTest.cpp
    )

target_link_libraries(bakTest bak gtest_main)

add_custom_target(testBak ALL COMMAND bakTest)
