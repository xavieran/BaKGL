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
    mouseEvent.hpp mouseEvent.cpp
    scene.hpp scene.cpp
    staticTTM.hpp staticTTM.cpp
    textBox.hpp
    widget.hpp widget.cpp
    window.hpp
)

target_link_libraries(gui
    bak
    com
    graphics
    guiInfo
    guiInventory
    xbak)

add_subdirectory(info)
add_subdirectory(inventory)
add_subdirectory(test)
