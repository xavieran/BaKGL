add_library(guiInventory
    containerDisplay.hpp
    inventoryScreen.hpp inventoryScreen.cpp
    inventorySlot.hpp
    itemArranger.hpp itemArranger.cpp
    modifiers.hpp
    shopDisplay.hpp
    shopItemSlot.hpp
)

add_subdirectory(test)

target_link_libraries(guiInventory
    bak
    com
    graphics
    gui
    )
