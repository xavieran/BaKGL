add_library(bak
    encounter/encounter.hpp encounter/encounter.cpp
    encounter/eventFlag.hpp encounter/eventFlag.cpp encounter/eventFlag.ipp
    encounter/background.hpp encounter/background.cpp
    encounter/block.hpp encounter/block.cpp
    encounter/combat.hpp encounter/combat.cpp encounter/combat.ipp
    encounter/gdsEntry.hpp encounter/gdsEntry.cpp encounter/gdsEntry.ipp 
    encounter/dialog.hpp encounter/dialog.cpp
    encounter/disable.hpp
    encounter/enable.hpp
    encounter/teleport.hpp encounter/teleport.cpp 
    encounter/town.hpp 
    encounter/trap.hpp
    encounter/zone.hpp encounter/zone.cpp
    camera.hpp
    condition.hpp condition.cpp
    constants.hpp
    character.hpp character.cpp
    container.hpp container.cpp
    coordinates.hpp coordinates.cpp
    dialog.hpp dialog.cpp
    dialogAction.hpp dialogAction.cpp
    dialogChoice.hpp dialogChoice.cpp
    dialogSources.hpp
    dialogTarget.hpp dialogTarget.cpp
    eventHandler.hpp
    fixedObject.hpp fixedObject.cpp
    font.hpp font.cpp
    gameData.hpp
    gameState.hpp
    hotspot.hpp hotspot.cpp
    hotspotRef.hpp hotspotRef.cpp
    inventory.hpp inventory.cpp
    money.hpp money.cpp
    party.hpp party.cpp
    palette.hpp palette.cpp
    objectInfo.hpp objectInfo.cpp
    resourceNames.hpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    skills.hpp skills.cpp
    textureFactory.hpp textureFactory.cpp
    worldClock.hpp worldClock.cpp
    worldFactory.hpp worldFactory.cpp
    zoneReference.hpp zoneReference.cpp
    zone.hpp
)

target_link_libraries(bak graphics xbak)
