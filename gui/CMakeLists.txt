add_library(gui
    IGuiElement.hpp
    gui.hpp
    gdsScene.hpp
    scene.hpp scene.cpp
    textBox.hpp
)

target_link_libraries(gui graphics bak xbak)
