add_library(bakEncounter
    encounter.hpp encounter.cpp
    encounterStore.hpp encounterStore.cpp
    eventFlag.hpp eventFlag.cpp eventFlag.ipp
    background.hpp background.cpp
    block.hpp block.cpp
    combat.hpp combat.cpp combat.ipp
    gdsEntry.hpp gdsEntry.cpp gdsEntry.ipp 
    dialog.hpp dialog.cpp
    disable.hpp disable.cpp
    enable.hpp enable.cpp
    teleport.hpp teleport.cpp 
    town.hpp town.cpp
    trap.hpp trap.cpp
    zone.hpp zone.cpp
)

target_link_libraries(bakEncounter bak)
