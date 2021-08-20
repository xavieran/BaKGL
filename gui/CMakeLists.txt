add_library(gui
    button.hpp
    clickButton.hpp
    colors.hpp
    cursor.hpp
    dialogRunner.hpp
    gdsScene.hpp
    hotspot.hpp
    label.hpp
    mainView.hpp
    scene.hpp scene.cpp
    textBox.hpp
    widget.hpp widget.cpp
    window.hpp
)

target_link_libraries(gui graphics bak xbak)
