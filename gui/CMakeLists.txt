add_library(gui
    gui.hpp
    scene.hpp scene.cpp
)

target_link_libraries(gui graphics bak)
