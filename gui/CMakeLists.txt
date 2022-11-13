enable_testing()

add_library(gui
    button.hpp
    clickButton.hpp
    colors.hpp
    cursor.hpp
    dragEvent.hpp dragEvent.cpp
    dialogRunner.hpp dialogRunner.cpp
    gdsScene.hpp gdsScene.cpp
    hotspot.hpp
    label.hpp
    mainView.hpp
    mainMenuScreen.hpp
    mouseEvent.hpp mouseEvent.cpp
    scene.hpp scene.cpp
    staticTTM.hpp staticTTM.cpp
    teleportScreen.hpp
    teleportDest.hpp
    textBox.hpp
    widget.hpp widget.cpp
    window.hpp
)

add_subdirectory(info)
add_subdirectory(inventory)

target_link_libraries(gui
    audio
    bak
    com
    graphics
    #guiInfo
    guiInventory
    xbak)

add_subdirectory(test)
