add_library(gui
    gui.hpp
    gdsScene.hpp
    scene.hpp scene.cpp
    textBox.hpp
)

target_link_libraries(gui graphics bak xbak)
