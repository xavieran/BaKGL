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
    dataTags.hpp
    dialog.hpp dialog.cpp
    dialogAction.hpp dialogAction.cpp
    dialogChoice.hpp dialogChoice.cpp
    dialogSources.hpp
    dialogTarget.hpp dialogTarget.cpp
    fixedObject.hpp fixedObject.cpp
    fileBufferFactory.hpp fileBufferFactory.cpp
    font.hpp font.cpp
    gameData.hpp gameData.cpp
    gameState.hpp
    haggle.hpp haggle.cpp
    hotspot.hpp hotspot.cpp
    hotspotRef.hpp hotspotRef.cpp
    image.hpp image.cpp
    imageStore.hpp imageStore.cpp
    inventory.hpp inventory.cpp
    itemInteractions.hpp itemInteractions.cpp
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
    screen.hpp screen.cpp
    screens.hpp
    shop.hpp shop.cpp
    sound.hpp sound.cpp
    soundStore.hpp soundStore.cpp
    skills.hpp skills.cpp
    tags.hpp tags.cpp
    temple.hpp temple.cpp
    textureFactory.hpp textureFactory.cpp
    worldClock.hpp worldClock.cpp
    worldItem.hpp worldItem.cpp
    worldFactory.hpp worldFactory.cpp
    zoneReference.hpp zoneReference.cpp
    zone.hpp
)

target_link_libraries(bak
    audio
    bakEncounter
    bakFile
    graphics)

#set_target_properties(
#    bak
#    PROPERTIES
#    CXX_CLANG_TIDY
#    "clang-tidy;-checks=-*,bugprone*,clang-analyzer*,cppcoreguidelines*,performance*,portability*")

add_subdirectory(encounter)
add_subdirectory(file)
add_subdirectory(test)
