add_library(game
    combatModelLoader.hpp combatModelLoader.cpp
    combatEncounterHandler.hpp combatEncounterHandler.cpp
    console.hpp console.cpp
    glyphStore.hpp glyphStore.cpp
    mapIcons.hpp mapIcons.cpp
    textAnimator.hpp textAnimator.cpp
    pitAnimator.hpp pitAnimator.cpp
    encounterHandler.hpp encounterHandler.cpp
    gameRunner.hpp gameRunner.cpp
    movementManager.hpp movementManager.cpp
    combat/types.hpp combat/types.cpp
    combat/actionQueue.hpp combat/actionQueue.cpp
    combat/combatManager.hpp combat/combatManager.cpp
    combat/combatStage.hpp combat/combatStage.cpp
    combat/gridAlgorithms.hpp combat/gridAlgorithms.cpp
    screens.hpp screens.cpp
    systems.hpp systems.cpp
    interactable/IInteractable.hpp
    interactable/all.cpp
    zoomManager.hpp zoomManager.cpp
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
