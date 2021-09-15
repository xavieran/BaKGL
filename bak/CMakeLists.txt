add_library(bak
    camera.hpp
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
    inventory.hpp
    resourceNames.hpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    skills.hpp skills.cpp
    systems.hpp
    textureFactory.hpp textureFactory.cpp
    town.hpp
    worldFactory.hpp worldFactory.cpp
)

target_link_libraries(bak graphics xbak)
