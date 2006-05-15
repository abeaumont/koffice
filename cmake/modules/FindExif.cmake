# - Try to find the libexif graphics library
# Once done this will define
#
#  LIBEXIF_FOUND - system has LIBEXIF
#  LIBEXIF_INCLUDE_DIR - the LIBEXIF include directory
#  LIBEXIF_LIBRARIES - Link these to use LIBEXIF
#  LIBEXIF_DEFINITIONS - Compiler switches required for using LIBEXIF
#


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
INCLUDE(UsePkgConfig)

PKGCONFIG(libexif _LibexifIncDir _LibexifLinkDir _LibexifLinkFlags _LibexifCflags)

if(_LibexifLinkFlags)
  # query pkg-config asking for a libexif >= 0.6.12
  EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.6.12 libexif RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
  if(_return_VALUE STREQUAL "0")
    set(LIBEXIF_FOUND TRUE)
  endif(_return_VALUE STREQUAL "0")
endif(_LibexifLinkFlags)

if (LIBEXIF_FOUND)
  set(LIBEXIF_INCLUDE_DIR ${_LibexifIncDir})
  set(LIBEXIF_LIBRARY ${_LibexifLinkFlags})
  if (NOT LIBEXIF_FIND_QUIETLY)
    message(STATUS "Found libexif: ${LIBEXIF_LIBRARY}")
  endif (NOT LIBEXIF_FIND_QUIETLY)
else (LIBEXIF_FOUND)
  if (LIBEXIF_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find libexif")
  endif (LIBEXIF_FIND_REQUIRED)
endif (LIBEXIF_FOUND)

MESSAGE(STATUS "LIBEXIF_INCLUDE_DIR : <${LIBEXIF_INCLUDE_DIR}> !!!! LIBEXIF_LIBRARY <${LIBEXIF_LIBRARY}>")

MARK_AS_ADVANCED(LIBEXIF_INCLUDE_DIR LIBEXIF_LIBRARY)

