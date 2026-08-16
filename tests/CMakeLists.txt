qt_add_executable(gucds_core_tests
    test_protocol.cpp
)

target_link_libraries(gucds_core_tests
    PRIVATE
        Qt6::Sql
        Qt6::Test
        gucds_core
)

add_test(NAME gucds_core_tests COMMAND gucds_core_tests)

qt_add_executable(gucds_can_tests
    test_can.cpp
)

target_link_libraries(gucds_can_tests
    PRIVATE
        Qt6::Test
        gucds_core
)

add_test(NAME gucds_can_tests COMMAND gucds_can_tests)

# This executable is intentionally not registered with CTest because it
# requires a connected PCAN-USB adapter and the STM32F103 test firmware.
qt_add_executable(gucds_pcan_hardware_test
    test_can_hardware.cpp
)

target_link_libraries(gucds_pcan_hardware_test
    PRIVATE
        Qt6::Core
        gucds_core
)

qt_add_executable(gucds_product_management_ui_tests
    test_product_management_ui.cpp
)

target_link_libraries(gucds_product_management_ui_tests
    PRIVATE
        Qt6::Test
        Qt6::Widgets
        gucds_widgets
)

add_test(NAME gucds_product_management_ui_tests COMMAND gucds_product_management_ui_tests)
set_tests_properties(gucds_product_management_ui_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

qt_add_executable(gucds_frequency_tension_parameters_ui_tests
    test_frequency_tension_parameters_ui.cpp
)

target_link_libraries(gucds_frequency_tension_parameters_ui_tests
    PRIVATE
        Qt6::Test
        Qt6::Widgets
        gucds_widgets
)

add_test(
    NAME gucds_frequency_tension_parameters_ui_tests
    COMMAND gucds_frequency_tension_parameters_ui_tests
)
set_tests_properties(gucds_frequency_tension_parameters_ui_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

qt_add_executable(gucds_dtu_config_ui_tests
    test_dtu_config_ui.cpp
)

target_link_libraries(gucds_dtu_config_ui_tests
    PRIVATE
        Qt6::Test
        Qt6::Widgets
        gucds_widgets
)

add_test(NAME gucds_dtu_config_ui_tests COMMAND gucds_dtu_config_ui_tests)
set_tests_properties(gucds_dtu_config_ui_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

qt_add_executable(gucds_mainwindow_ui_tests
    test_mainwindow_ui.cpp
    ../src/app/applicationtranslator.cpp
    ../src/app/applicationtranslator.h
    ../src/app/mainwindow.cpp
    ../src/app/mainwindow.h
    ../src/app/spectrumplotwidget.cpp
    ../src/app/spectrumplotwidget.h
)

target_include_directories(gucds_mainwindow_ui_tests
    PRIVATE
        ../src/app
)

target_link_libraries(gucds_mainwindow_ui_tests
    PRIVATE
        Qt6::Test
        Qt6::Widgets
        gucds_core
        gucds_widgets
)

qt_add_resources(gucds_mainwindow_ui_tests "gucds_test_translations"
    PREFIX
        "/i18n"
    FILES
        ../translations/gucds_en_US.ts
)

add_test(NAME gucds_mainwindow_ui_tests COMMAND gucds_mainwindow_ui_tests)
set_tests_properties(gucds_mainwindow_ui_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
