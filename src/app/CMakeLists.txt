qt_add_executable(gucds_app
    applicationtranslator.cpp
    applicationtranslator.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    spectrumplotwidget.cpp
    spectrumplotwidget.h
)

if(WIN32)
    configure_file(gucds_app.rc.in "${CMAKE_CURRENT_BINARY_DIR}/gucds_app.rc" @ONLY)
    target_sources(gucds_app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/gucds_app.rc")
endif()

target_link_libraries(gucds_app
    PRIVATE
        Qt6::Widgets
        gucds_core
        gucds_widgets
)

target_compile_definitions(gucds_app
    PRIVATE
        GUCDS_APP_VERSION="${PROJECT_VERSION}"
)

set_target_properties(gucds_app PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(APPLE AND GUCDS_QT_PLUGIN_DIR)
    add_custom_command(TARGET gucds_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${GUCDS_QT_PLUGIN_DIR}"
            "$<TARGET_FILE_DIR:gucds_app>/plugins"
        VERBATIM
    )

    file(GENERATE
        OUTPUT "$<TARGET_FILE_DIR:gucds_app>/qt.conf"
        CONTENT "[Paths]\nPlugins = plugins\n"
    )
endif()
