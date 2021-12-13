enable_testing()

include(GoogleTest)

add_executable(guiInventoryTest
    itemArrangerTest.cpp
    )

target_link_libraries(guiInventoryTest
    #stdc++
    #m
    gui
    guiInventory
    gtest_main)

gtest_discover_tests(guiInventoryTest
    TEST_SUFFIX .guiInventoryTest
)

add_test(NAME testGuiInventory COMMAND guiInventoryTest)
