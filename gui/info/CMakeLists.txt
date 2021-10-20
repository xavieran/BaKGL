add_library(guiInfo
    infoScreen.hpp
    portrait.hpp
    ratings.hpp
    skills.hpp
)

target_link_libraries(guiInfo
    bak
    com
    graphics
    gui
    )
