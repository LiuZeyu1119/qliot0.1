qt_add_executable(gucds_import_labview_data
    import_labview_data.cpp
)

target_link_libraries(gucds_import_labview_data
    PRIVATE
        Qt6::Core
        gucds_core
)
