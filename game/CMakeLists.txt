add_library(game
    combatModelLoader.hpp combatModelLoader.cpp
    combatEncounterHandler.hpp combatEncounterHandler.cpp
    console.hpp console.cpp
    encounterHandler.hpp encounterHandler.cpp
    gameRunner.hpp gameRunner.cpp
    combat/types.hpp combat/types.cpp
    combat/actionQueue.hpp combat/actionQueue.cpp
    combat/combatManager.hpp combat/combatManager.cpp
    combat/gridAlgorithms.hpp combat/gridAlgorithms.cpp
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

add_subdirectory(combat/test)
