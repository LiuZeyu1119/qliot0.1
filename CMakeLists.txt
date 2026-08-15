cmake_minimum_required(VERSION 3.21)

if(CMAKE_HOST_APPLE)
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE FILEPATH "C++ compiler" FORCE)
    endif()

    set(_gucds_homebrew_qt_dir "/opt/homebrew/opt/qtbase/lib/cmake/Qt6")
    if(EXISTS "${_gucds_homebrew_qt_dir}/Qt6Config.cmake")
        set(Qt6_DIR "${_gucds_homebrew_qt_dir}" CACHE PATH "Qt 6 package directory" FORCE)
    endif()
endif()

project(GUCDSQt
    VERSION 5.7.4
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_HOST_APPLE AND EXISTS "/opt/homebrew/opt/qtbase/share/qt/plugins")
    set(_gucds_default_qt_plugin_dir "/opt/homebrew/opt/qtbase/share/qt/plugins")
endif()
set(GUCDS_QT_PLUGIN_DIR "${_gucds_default_qt_plugin_dir}" CACHE PATH "Qt plugin directory copied beside the application")

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Sql Widgets Test)
qt_standard_project_setup(REQUIRES 6.8)

add_subdirectory(src/core)
add_subdirectory(src/widgets)
add_subdirectory(src/app)
add_subdirectory(src/tools)

enable_testing()
add_subdirectory(tests)

qt_add_resources(gucds_app "gucds_translations"
    PREFIX
        "/i18n"
    FILES
        translations/gucds_en_US.ts
)

qt_add_resources(gucds_app "gucds_app_icons"
    PREFIX
        "/icons"
    BASE
        "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        icon.png
)
