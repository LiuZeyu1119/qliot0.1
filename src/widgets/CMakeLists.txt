qt_add_library(gucds_widgets STATIC
    dtuconfigdialog.cpp
    include/gucds/widgets/dtuconfigdialog.h
    frequencytensionparameterdialog.cpp
    include/gucds/widgets/frequencytensionparameterdialog.h
    productmanagementdialog.cpp
    include/gucds/widgets/productmanagementdialog.h
)

target_include_directories(gucds_widgets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gucds_widgets
    PUBLIC
        Qt6::Widgets
        gucds_core
)
