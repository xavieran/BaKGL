add_library(gui
    button.hpp
    bookPlayer.hpp bookPlayer.cpp
    clickButton.hpp
    colors.hpp
    cursor.hpp
    dialogRunner.hpp dialogRunner.cpp
    dynamicTTM.hpp dynamicTTM.cpp
    gdsScene.hpp gdsScene.cpp
    hotspot.hpp
    label.hpp
    mainView.hpp
    mainMenuScreen.hpp
    scene.hpp scene.cpp
    staticTTM.hpp staticTTM.cpp
    teleportScreen.hpp
    teleportDest.hpp
    textBox.hpp
    textInput.hpp
    window.hpp
)

add_subdirectory(camp)
add_subdirectory(core)
add_subdirectory(info)
add_subdirectory(inventory)

target_link_libraries(gui
    audio
    bak
    com
    graphics
    guiCamp
    guiCore
    guiInventory)
