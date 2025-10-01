set(
    extendedcolorselector_SOURCES
    ColorState.cpp
    ExtendedChannelPlane.cpp
    ExtendedChannelSlider.cpp
    ExtendedColorModel.cpp
    ExtendedColorSelectorDock.cpp
    ExtendedColorSelectorPlugin.cpp
    ExtendedShape.cpp
)

kis_add_library(extendedcolorselector MODULE ${extendedcolorselector_SOURCES})

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
