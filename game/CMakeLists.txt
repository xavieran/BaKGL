add_library(game
    combatModelLoader.hpp combatModelLoader.cpp
    combatEncounterHandler.hpp combatEncounterHandler.cpp
    console.hpp console.cpp
    encounterHandler.hpp encounterHandler.cpp
    gameRunner.hpp gameRunner.cpp
    screens.hpp screens.cpp
    systems.hpp systems.cpp
    interactable/IInteractable.hpp
    interactable/all.cpp
    )

target_link_libraries(game
    audio
    bak
    com
    glm
    graphics
    gui
    imgui)
