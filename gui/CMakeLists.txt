add_library(gui
    button.hpp
    bookPlayer.hpp bookPlayer.cpp
    clickButton.hpp
    colors.hpp
    cursor.hpp
    dialogRunner.hpp dialogRunner.cpp
    dynamicTTM.hpp dynamicTTM.cpp
    guiManager.hpp guiManager.cpp
    gdsScene.hpp gdsScene.cpp
    hotspot.hpp
    label.hpp
    mainView.hpp
    mainMenuScreen.hpp
    scene.hpp scene.cpp
    staticTTM.hpp staticTTM.cpp
    teleportScreen.hpp
    teleportDest.hpp
    textBox.hpp
    textInput.hpp
    window.hpp

    camp/campScreen.hpp
    camp/clock.hpp
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
