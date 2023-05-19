enable_testing()

add_library(guiCore
    doubleClickStateMachine.hpp doubleClickStateMachine.cpp
    dragEvent.hpp dragEvent.cpp
    draggable.hpp
    keyEvent.hpp keyEvent.cpp
    mouseEvent.hpp mouseEvent.cpp
    widget.hpp widget.cpp
)

target_link_libraries(guiCore
    audio
    bak
    com
    graphics)

add_subdirectory(test)
