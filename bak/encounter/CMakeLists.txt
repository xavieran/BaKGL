add_library(bakEncounter
    encounter.hpp encounter.cpp
    eventFlag.hpp eventFlag.cpp eventFlag.ipp
    background.hpp background.cpp
    block.hpp block.cpp
    combat.hpp combat.cpp combat.ipp
    gdsEntry.hpp gdsEntry.cpp gdsEntry.ipp 
    dialog.hpp dialog.cpp
    disable.hpp
    enable.hpp
    teleport.hpp teleport.cpp 
    town.hpp 
    trap.hpp
    zone.hpp zone.cpp
)

target_link_libraries(bakEncounter bak xbak)
