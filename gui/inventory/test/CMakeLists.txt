add_executable(guiInventoryTest
    itemArrangerTest.cpp
    )

target_link_libraries(guiInventoryTest
    stdc++
    m
    gui
    guiInventory
    gtest_main)

add_custom_target(testGuiInventory ALL COMMAND guiInventoryTest)
