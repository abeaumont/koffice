include_directories( 
    ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${PIGMENT_INCLUDES}
)

if(KDE4_BUILD_TESTS)
    # only with this definition will the KOPAGEAPP_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)
add_subdirectory(tools)

### kopageapp ###
set(kopageapp_LIB_SRCS
    KoPACanvas.cpp
    KoPADocument.cpp
    KoPADocumentModel.cpp
    KoPADocumentStructureDocker.cpp
    KoPALoadingContext.cpp
    KoPAMasterPage.cpp
    KoPAMasterShapeProxy.cpp
    KoPAOdfPageSaveHelper.cpp
    KoPAPageContainerModel.cpp
    KoPAPage.cpp
    KoPAPageThumbnailModel.cpp
    KoPAPastePage.cpp
    KoPAPixmapCache.cpp
    KoPAPrintJob.cpp
    KoPASavingContext.cpp
    KoPATextPage.cpp
    KoPAUtil.cpp
    KoPAView.cpp
    KoPAViewMode.cpp
    KoPAViewModeNormal.cpp
    KoShapeTraversal.cpp
    commands/KoPAPageInsertCommand.cpp
    commands/KoPAPageDeleteCommand.cpp
    commands/KoPAPageMoveCommand.cpp
    commands/KoPAChangeMasterPageCommand.cpp
    commands/KoPAChangePageLayoutCommand.cpp
    commands/KoPADisplayMasterShapesCommand.cpp
    commands/KoPADisplayMasterBackgroundCommand.cpp
    dialogs/KoPAMasterPageDialog.cpp
    dialogs/KoPAPageLayoutDialog.cpp
    dialogs/KoPAConfigureDialog.cpp

    tools/backgroundTool/KoPABackgroundTool.cpp
    tools/backgroundTool/KoPABackgroundToolFactory.cpp
    tools/backgroundTool/KoPABackgroundToolWidget.cpp
    tools/backgroundTool/KoPAMasterPageDocker.cpp
)

kde4_add_ui_files(kopageapp_LIB_SRCS
    tools/backgroundTool/BackgroundToolWidget.ui
)

kde4_add_library(kopageapp SHARED ${kopageapp_LIB_SRCS})

target_link_libraries(kopageapp komain)
target_link_libraries(kopageapp LINK_INTERFACE_LIBRARIES komain)

set_target_properties(kopageapp PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kopageapp  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kopageapp_export.h
    KoPACanvas.h
    KoPADocument.h
    KoPageApp.h
    KoPALoadingContext.h
    KoPAPageContainerModel.h
    KoPAUtil.h
    KoPAView.h
    KoPAViewMode.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/private COMPONENT Devel
)
