set(
    extendedcolorselector_SOURCES
    EXChannelPlane.cpp
    EXChannelSlider.cpp
    EXColorModel.cpp
    EXColorModelSwitchers.cpp
    EXColorPatchPopup.cpp
    EXColorSelectorDock.cpp
    EXColorSelectorPlugin.cpp
    EXColorState.cpp
    EXDynamicRangeSlider.cpp
    EXEditable.cpp
    EXGamutClipping.cpp
    EXKoColorConverter.cpp
    EXPortableColorSelector.cpp
    EXSettings.cpp
    EXSettingsDialog.cpp
    EXSettingsState.cpp
    EXShape.cpp
    EXUtils.cpp
    KisGLImageF16.cpp
    KisGLImageWidget.cpp
    OptionalColorPicker.cpp
)

set(CMAKE_VERBOSE_MAKEFILE ON)

qt5_add_resources(extendedcolorselector_RESOURCES resources.qrc kis_gl_image_widget.qrc)

kis_add_library(extendedcolorselector MODULE ${extendedcolorselector_SOURCES} ${extendedcolorselector_RESOURCES})

generate_export_header(extendedcolorselector BASE_NAME extendedcolorselector)

if(QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(extendedcolorselector kritaui Qt6::OpenGLWidgets)
else()
    target_link_libraries(extendedcolorselector PRIVATE kritaui)
endif()

target_include_directories(extendedcolorselector PRIVATE
    ${CMAKE_SOURCE_DIR}/libs
)

set_target_properties(extendedcolorselector PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)
