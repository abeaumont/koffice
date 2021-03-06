include(CheckIncludeFiles)

macro_optional_find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXML2" "LibXml2 library" "http://xmlsoft.org/" FALSE "" "Required by the xlslt filter")

macro_optional_find_package(LibXslt)
macro_log_feature(LIBXSLT_FOUND "LibXslt" "LibXslt Library" "http://xmlsoft.org/XSLT/" FALSE "" "Required by the xslt filter")

# for xsltfilter:
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
# for xpdf:
check_include_files(ndir.h HAVE_NDIR_H)
check_include_files(sys/dir.h HAVE_SYS_DIR_H)
check_include_files(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_files(dirent.h HAVE_DIRENT_H)
# others:
check_include_files(ieeefp.h HAVE_IEEEFP_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-filters.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-filters.h )

set(liboofilter_SRCS 
	${CMAKE_SOURCE_DIR}/filters/liboofilter/ooutils.cc 
	${CMAKE_SOURCE_DIR}/filters/liboofilter/liststylestack.cc
	)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


set(KWMF_INCLUDES ${CMAKE_SOURCE_DIR}/filters/libkowmf)

add_subdirectory(libmso)

macro_optional_add_subdirectory( libkowmf )
macro_optional_add_subdirectory( libmsooxml )

if (SHOULD_BUILD_KWORD)
    add_subdirectory(kword)
endif (SHOULD_BUILD_KWORD)

if(SHOULD_BUILD_SHOWCASE)
    add_subdirectory(showcase)
endif(SHOULD_BUILD_SHOWCASE)

if(SHOULD_BUILD_KCELLS)
    add_subdirectory(kcells)
endif(SHOULD_BUILD_KCELLS)

if(SHOULD_BUILD_ARTWORK)
  add_subdirectory(artwork)
endif(SHOULD_BUILD_ARTWORK)

if(LIBXML2_FOUND AND LIBXSLT_FOUND AND NOT WIN32 AND NOT QT_MAC_USE_COCOA)
    add_subdirectory(xsltfilter)
endif(LIBXML2_FOUND AND LIBXSLT_FOUND AND NOT WIN32 AND NOT QT_MAC_USE_COCOA)

########### install files ###############

