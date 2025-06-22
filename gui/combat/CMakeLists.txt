add_library(guiCombat
    combatScreen.hpp combatScreen.cpp
)

target_link_libraries(guiCombat
    bak
    com
    graphics
    gui
    )
