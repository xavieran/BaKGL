list(APPEND APP_BINARIES
    dialog_explorer
    display_dialog
    display_encounters
    display_fo
    display_fmap
    display_gds
    display_object
    display_save
    display_tile
    display_ttm
    instancing
    main3d
    show_scene
    show_imgui
    bmx_explorer
)

set(APP_LIBS
    bak
    com
    graphics
    gui
    imgui
    xbak
)

set(CMAKE_CXX_CLANG_TIDY "")

foreach (APP_BIN IN LISTS APP_BINARIES)
    add_executable (${APP_BIN} ${APP_BIN}.cpp)
    target_link_libraries(${APP_BIN}
        ${LINK_UNIX_LIBRARIES}
        ${LINK_3D_LIBRARIES}
        ${SDL_LIBRARIES}
        ${APP_LIBS}
    )

endforeach()


