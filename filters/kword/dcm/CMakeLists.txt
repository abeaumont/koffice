set(dcmimport_PART_SRCS dcmimport.cpp DCMImportDialog.cpp )
kde4_add_ui_files(dcmimport_PART_SRCS DCMImportDialog.ui )

INCLUDE_DIRECTORIES( ${DCMTK_INCLUDE_DIR} )
add_definitions(-DHAVE_CONFIG_H)

kde4_add_plugin(dcmimport ${dcmimport_PART_SRCS})

target_link_libraries(dcmimport kwordexportfilters ${DCMTK_LIBRARIES})

install(TARGETS dcmimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  kword_dcm_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
