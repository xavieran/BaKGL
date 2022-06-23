add_library(game
    console.hpp
    gameRunner.hpp
    systems.hpp
    interactable/IInteractable.hpp
    interactable/building.hpp
    interactable/chest.hpp
    interactable/factory.hpp interactable/factory.cpp
    interactable/generic.hpp
    interactable/ladder.hpp
    interactable/tomb.hpp
    )

target_link_libraries(game
    audio
    bak
    com
    glm
    graphics
    gui
    imgui)
