add_library(bak
    camera.hpp
    condition.hpp condition.cpp
    constants.hpp
    character.hpp character.cpp
    container.hpp container.cpp
    coordinates.hpp coordinates.cpp
    dialog.hpp dialog.cpp
    dialogAction.hpp dialogAction.cpp
    dialogSources.hpp
    dialogTarget.hpp dialogTarget.cpp
    encounter.hpp encounter.cpp
    eventHandler.hpp
    fixedObject.hpp fixedObject.cpp
    font.hpp font.cpp
    gameData.hpp
    gameState.hpp
    hotspot.hpp hotspot.cpp
    inventory.hpp inventory.cpp
    party.hpp party.cpp
    objectInfo.hpp objectInfo.cpp
    resourceNames.hpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    skills.hpp skills.cpp
    systems.hpp
    textureFactory.hpp textureFactory.cpp
    town.hpp
    worldClock.hpp worldClock.cpp
    worldFactory.hpp worldFactory.cpp
)

target_link_libraries(bak graphics xbak)
