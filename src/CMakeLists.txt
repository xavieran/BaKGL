
add_library(bak
    camera.hpp
    constants.hpp
    coordinates.hpp
    dialog.hpp dialog.cpp
    dialogSources.hpp
    dialogTarget.hpp dialogTarget.cpp
    encounter.hpp
    eventHandler.hpp
    fixedObject.hpp fixedObject.cpp
    gameData.hpp
    glm.hpp
    hotspot.hpp hotspot.cpp
    inputHandler.hpp
    inventory.hpp
    logger.hpp logger.cpp
    ostream.hpp
    resourceNames.hpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    systems.hpp
    texture.hpp
    town.hpp
    worldFactory.hpp
)

target_link_libraries(bak xbak)
