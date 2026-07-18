enable_testing()

add_library(bak
    save/party.hpp save/party.cpp
    save/character.hpp save/character.cpp
    save/world.hpp save/world.cpp
    save/combat.hpp save/combat.cpp
    save/containers.hpp save/containers.cpp
    IContainer.hpp IContainer.cpp
    backgroundSounds.hpp backgroundSounds.cpp
    bard.hpp bard.cpp
    book.hpp book.cpp
    camera.hpp camera.cpp
    camp.hpp camp.cpp
    collision.hpp collision.cpp
    partyChangeCache.hpp partyChangeCache.cpp
    chapterTransitions.hpp chapterTransitions.cpp
    combat/combat.hpp combat/combat.cpp
    combat/calculations.hpp combat/calculations.cpp
    combat/spellEffects.hpp combat/spellEffects.cpp
    combat/combatModel.hpp combat/combatModel.cpp
    combat/retreat.hpp combat/retreat.cpp
    combat/types.hpp combat/types.cpp
    condition.hpp condition.cpp
    constants.hpp constants.cpp
    cutscenes.hpp cutscenes.cpp
    character.hpp character.cpp
    container.hpp container.cpp
    coordinates.hpp coordinates.cpp
    dataTags.hpp
    dialog.hpp dialog.cpp
    dialogReplacements.hpp dialogReplacements.cpp
    dialogAction.hpp dialogAction.cpp
    dialogChoice.hpp dialogChoice.cpp
    dialogJson.hpp dialogJson.cpp
    dialogSources.hpp dialogSources.cpp
    dialogTarget.hpp dialogTarget.cpp
    entityType.hpp entityType.cpp
    fixedObject.hpp fixedObject.cpp
    fileBufferFactory.hpp fileBufferFactory.cpp
    fmap.hpp fmap.cpp
    font.hpp font.cpp
    gameData.hpp gameData.cpp
    gameState.hpp gameState.cpp
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
    lua/core.hpp lua/core.cpp lua/hooks.hpp lua/hooks.cpp lua/types.hpp lua/types.cpp
    model.hpp model.cpp
    money.hpp money.cpp
    monster.hpp monster.cpp
    movement.hpp movement.cpp
    party.hpp party.cpp
    palette.hpp palette.cpp
    objectInfo.hpp objectInfo.cpp
    ramp.hpp ramp.cpp
    random.hpp random.cpp
    resourceNames.hpp resourceNames.cpp
    saveManager.hpp saveManager.cpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screen.hpp screen.cpp
    shop.hpp shop.cpp
    skills.hpp skills.cpp
    sound.hpp sound.cpp
    soundStore.hpp soundStore.cpp
    spells.hpp spells.cpp
    spriteRenderer.hpp spriteRenderer.cpp
    startupFiles.hpp startupFiles.cpp
    tags.hpp tags.cpp
    temple.hpp temple.cpp
    ttmRenderer.hpp ttmRenderer.cpp
    ttmRunner.hpp ttmRunner.cpp
    textureFactory.hpp textureFactory.cpp
    textVariableStore.hpp textVariableStore.cpp
    time.hpp time.cpp
    timeExpiringState.hpp timeExpiringState.cpp
    trap.hpp trap.cpp
    worldClock.hpp worldClock.cpp
    worldItem.hpp worldItem.cpp
    worldFactory.hpp worldFactory.cpp
    zoneReference.hpp zoneReference.cpp
    zone.hpp zone.cpp
)

add_subdirectory(encounter)
add_subdirectory(file)
add_subdirectory(state)
add_subdirectory(test)
add_subdirectory(combat/test)

target_link_libraries(bak
    audio
    bakEncounter
    bakFile
    bakState
    graphics
    ${LUA_LIBRARIES}
    LuaBridge)

#set_target_properties(
#    bak
#    PROPERTIES
#    CXX_CLANG_TIDY
#    "clang-tidy;-checks=-*,bugprone*,clang-analyzer*,cppcoreguidelines*,performance*,portability*,-cppcoreguidelines-avoid-magic-numbers,-cppcoreguidelines-pro-type-union-access,-cppcoreguidelines-pro-bounds-constant-array-index,-performance-avoid-endl,-bugprone-easily-swappable-parameters,-bugprone-branch-clone")
