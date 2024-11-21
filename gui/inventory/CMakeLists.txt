add_library(guiInventory
    containerDisplay.hpp containerDisplay.cpp
    details.hpp details.cpp
    equipmentSlot.hpp equipmentSlot.cpp
    inventoryScreen.hpp inventoryScreen.cpp
    inventorySlot.hpp inventorySlot.cpp
    itemArranger.hpp itemArranger.cpp
    modifiers.hpp modifiers.cpp
    shopDisplay.hpp shopDisplay.cpp
    shopItemSlot.hpp shopItemSlot.cpp
    splitStackDialog.hpp splitStackDialog.cpp
)

add_subdirectory(test)

target_link_libraries(guiInventory
    bak
    com
    graphics
    gui
    )
