set(
    extendedcolorselector_SOURCES
    EXChannelPlane.cpp
    EXChannelSlider.cpp
    EXColorModel.cpp
    EXColorModelSwitchers.cpp
    EXColorSelectorDock.cpp
    EXColorSelectorPlugin.cpp
    EXColorState.cpp
    EXEditable.cpp
    EXGamutClipping.cpp
    EXKoColorConverter.cpp
    EXPortableColorSelector.cpp
    EXSettings.cpp
    EXSettingsDialog.cpp
    EXSettingsState.cpp
    EXShape.cpp
    EXUtils.cpp
    OptionalColorPicker.cpp
)

qt5_add_resources(extendedcolorselector_RESOURCES resources.qrc)

kis_add_library(extendedcolorselector MODULE ${extendedcolorselector_SOURCES} ${extendedcolorselector_RESOURCES})

generate_export_header(extendedcolorselector BASE_NAME extendedcolorselector)

# target_compile_features(extendedcolorselector PRIVATE cxx_std_20)
target_link_libraries(extendedcolorselector PRIVATE kritaui)
target_include_directories(extendedcolorselector PRIVATE
    ${CMAKE_SOURCE_DIR}/libs
)

set_target_properties(extendedcolorselector PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)
