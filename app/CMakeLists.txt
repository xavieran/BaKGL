list(APPEND APP_BINARIES
    dialog_explorer
    display_dialog
    display_fo
    display_gds
    display_object
    display_save
    display_tile
    display_ttm
    main2d
    main3d
    show_scene
    show_imgui
    bmx_explorer
)

set(LINK_3D_LIBRARIES 
    bak
    com
    glfw
    graphics
    gui
    imgui
    xbak
    ${GLEW_LIBRARIES} 
    ${GLM_LIBRARIES}
    ${OPENGL_LIBRARY}
)

foreach (APP_BIN IN LISTS APP_BINARIES)
    add_executable (${APP_BIN} ${APP_BIN}.cpp)
    target_link_libraries(${APP_BIN}
        stdc++
        m
        dl
        pthread 
        ${LINK_3D_LIBRARIES}
        SDL
        ${APP_INCLUDES})
endforeach()


