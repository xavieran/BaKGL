add_library(bak
    IContainer.hpp IContainer.cpp
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
    inventoryItem.hpp inventoryItem.cpp
    keyContainer.hpp keyContainer.cpp
    layout.hpp layout.cpp
    money.hpp money.cpp
    party.hpp party.cpp
    palette.hpp palette.cpp
    objectInfo.hpp objectInfo.cpp
    resourceNames.hpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    shop.hpp shop.cpp
    skills.hpp skills.cpp
    textureFactory.hpp textureFactory.cpp
    worldClock.hpp worldClock.cpp
    worldFactory.hpp worldFactory.cpp
    zoneReference.hpp zoneReference.cpp
    zone.hpp
)

target_link_libraries(bak
    bakEncounter
    graphics
    xbak)

add_subdirectory(encounter)
add_subdirectory(test)
