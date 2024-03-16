list(APPEND APP_BINARIES
    decomp_ttm
    dialog_explorer
    display_book
    display_dialog
    display_encounters
    display_fo
    display_fmap
    display_gds
    display_object
    display_save
    display_sound
    display_spells
    display_tbl
    display_tile
    display_traps
    display_ttm
    instancing
    main3d
    play_book
    play_ttm
    show_scene
    show_imgui
    bmx_explorer
)

set(APP_LIBS
    bak
    com
    game
    graphics
    gui
    imgui
)


foreach (APP_BIN IN LISTS APP_BINARIES)
    add_executable (${APP_BIN} ${APP_BIN}.cpp)
    set_target_properties(${APP_BIN} PROPERTIES CMAKE_CXX_CLANG_TIDY "")
    target_link_libraries(${APP_BIN}
        ${LINK_UNIX_LIBRARIES}
        ${LINK_3D_LIBRARIES}
        #${SDL_LIBRARIES}
        ${APP_LIBS}
    )
endforeach()

install(TARGETS main3d)


