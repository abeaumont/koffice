########### next target ###############

set(wpexport_PART_SRCS wpexport.cc wp5.cc wp6.cc)

kde4_add_plugin(wpexport ${wpexport_PART_SRCS})

target_link_libraries(wpexport kwordexportfilters)

########### install files ###############

install(TARGETS wpexport  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kword_wp_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
