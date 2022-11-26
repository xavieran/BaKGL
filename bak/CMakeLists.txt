enable_testing()

add_library(bak
    IContainer.hpp IContainer.cpp
    bard.hpp bard.cpp
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
    fixedObject.hpp fixedObject.cpp
    font.hpp font.cpp
    gameData.hpp gameData.cpp
    gameState.hpp
    haggle.hpp haggle.cpp
    hotspot.hpp hotspot.cpp
    hotspotRef.hpp hotspotRef.cpp
    inventory.hpp inventory.cpp
    inventoryItem.hpp inventoryItem.cpp
    keyContainer.hpp keyContainer.cpp
    layout.hpp layout.cpp
    lock.hpp lock.cpp
    model.hpp model.cpp
    money.hpp money.cpp
    monster.hpp monster.cpp
    party.hpp party.cpp
    palette.hpp palette.cpp
    objectInfo.hpp objectInfo.cpp
    random.hpp random.cpp
    resourceNames.hpp
    save.hpp save.cpp
    saveFile.hpp saveFile.cpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    shop.hpp shop.cpp
    sound.hpp sound.cpp
    soundStore.hpp soundStore.cpp
    skills.hpp skills.cpp
    temple.hpp temple.cpp
    textureFactory.hpp textureFactory.cpp
    worldClock.hpp worldClock.cpp
    worldFactory.hpp worldFactory.cpp
    zoneReference.hpp zoneReference.cpp
    zone.hpp
)

target_link_libraries(bak
    audio
    bakEncounter
    graphics
    xbak)

add_subdirectory(encounter)
add_subdirectory(test)
