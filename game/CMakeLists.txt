add_library(game
    console.hpp console.cpp
    gameRunner.hpp gameRunner.cpp
    systems.hpp systems.cpp
    interactable/IInteractable.hpp
    interactable/building.hpp interactable/building.cpp
    interactable/chest.hpp interactable/chest.cpp
    interactable/combatant.hpp interactable/combatant.cpp
    interactable/door.hpp interactable/door.cpp
    interactable/factory.hpp interactable/factory.cpp
    interactable/generic.hpp interactable/generic.cpp
    interactable/ladder.hpp interactable/ladder.cpp
    interactable/pit.hpp interactable/pit.cpp
    interactable/tomb.hpp interactable/tomb.cpp
    )

target_link_libraries(game
    audio
    bak
    com
    glm
    graphics
    gui
    imgui)
