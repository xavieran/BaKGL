add_library(bak
    camera.hpp
    constants.hpp
    coordinates.hpp coordinates.cpp
    dialog.hpp dialog.cpp
    dialogSources.hpp
    dialogTarget.hpp dialogTarget.cpp
    encounter.hpp encounter.cpp
    eventHandler.hpp
    fixedObject.hpp fixedObject.cpp
    font.hpp font.cpp
    gameData.hpp
    hotspot.hpp hotspot.cpp
    inventory.hpp
    resourceNames.hpp
    scene.hpp scene.cpp
    sceneData.hpp sceneData.cpp
    screens.hpp
    systems.hpp
    textureFactory.hpp textureFactory.cpp
    town.hpp
    worldFactory.hpp worldFactory.cpp
)

target_link_libraries(bak graphics xbak)
