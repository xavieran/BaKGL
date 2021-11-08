add_library(guiInventory
    inventoryScreen.hpp
)

add_subdirectory(test)

target_link_libraries(guiInventory
    bak
    com
    graphics
    gui
    )
