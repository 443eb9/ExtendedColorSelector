add_subdirectory(src)

set(install_dir ${CMAKE_INSTALL_PREFIX}/extended_color_selector)
set(lib_files $<TARGET_FILE:extendedcolorselector>)

install(FILES ${lib_files} DESTINATION ${install_dir}/extended_color_selector/lib)
install(DIRECTORY python/ DESTINATION ${install_dir}/extended_color_selector)
install(FILES extended_color_selector.desktop DESTINATION ${install_dir})
install(FILES $<TARGET_FILE:extendedcolorselector> DESTINATION ${install_dir}/extended_color_selector/lib)
