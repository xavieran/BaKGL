add_library(gui
    actors.hpp actors.cpp
    animator.hpp animator.cpp
    backgrounds.hpp backgrounds.cpp
    button.hpp button.cpp
    bookPlayer.hpp bookPlayer.cpp
    callbackDelay.hpp callbackDelay.cpp
    centeredImage.hpp centeredImage.cpp
    clickButton.hpp clickButton.cpp
    colors.hpp
    compass.hpp compass.cpp
    contents.hpp contents.cpp
    cursor.hpp cursor.cpp
    dialogDisplay.hpp dialogDisplay.cpp
    dialogRunner.hpp dialogRunner.cpp
    dynamicTTM.hpp dynamicTTM.cpp
    fadeScreen.hpp fadeScreen.cpp
    fontManager.hpp fontManager.cpp
    fullMap.hpp fullMap.cpp
    guiManager.hpp guiManager.cpp
    gdsScene.hpp gdsScene.cpp
    hotspot.hpp hotspot.cpp
    icons.hpp icons.cpp
    label.hpp label.cpp
    mainView.hpp mainView.cpp
    mainMenuScreen.hpp mainMenuScreen.cpp
    preferencesScreen.hpp preferencesScreen.cpp
    saveScreen.hpp saveScreen.cpp
    repair.hpp repair.cpp
    scene.hpp scene.cpp
    screenStack.hpp screenStack.cpp
    scrollBar.hpp scrollBar.cpp
    scrollView.hpp
    staticTTM.hpp staticTTM.cpp
    teleportScreen.hpp
    teleportDest.hpp
    textBox.hpp textBox.cpp
    textInput.hpp
    window.hpp

    cast/castScreen.hpp cast/castScreen.cpp
    cast/powerRing.hpp cast/powerRing.cpp
    cast/symbol.hpp cast/symbol.cpp

    camp/campScreen.hpp camp/campScreen.cpp
    camp/clock.hpp camp/clock.cpp

    temple/temple.hpp temple/temple.cpp
    temple/cureScreen.hpp temple/cureScreen.cpp
)

add_subdirectory(core)
add_subdirectory(info)
add_subdirectory(inventory)

target_link_libraries(gui
    audio
    bak
    com
    graphics
    guiCore
    guiInventory)

#set_target_properties(
#    gui 
#    PROPERTIES
#    CXX_CLANG_TIDY
#    "clang-tidy;-checks=-*,bugprone*,clang-analyzer*,cppcoreguidelines*,performance*,portability*,-cppcoreguidelines-avoid-magic-numbers,-cppcoreguidelines-pro-type-union-access,-perfomance-avoid-endl,-bugprone-easily-swappable-parameters")
