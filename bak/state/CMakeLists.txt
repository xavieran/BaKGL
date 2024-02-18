add_library(bakState
    customStateChoice.hpp customStateChoice.cpp
    dialog.hpp dialog.cpp
    encounter.hpp encounter.cpp
    event.hpp event.cpp
    item.hpp item.cpp
    lock.hpp lock.cpp
    offsets.hpp
    skill.hpp skill.cpp
    temple.hpp temple.cpp
)

target_link_libraries(bakState bak bakEncounter)
