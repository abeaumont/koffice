# - Try to find the freetype library
# Once done this will define
#
#  FREETYPE_FOUND - system has Fontconfig
#  FREETYPE_INCLUDE_DIR - the FONTCONFIG include directory
#  FREETYPE_LIBRARIES - Link these to use FREETYPE
#

if (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)

  # in cache already
  set(FREETYPE_FOUND TRUE)

else (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)

  FIND_PROGRAM(FREETYPECONFIG_EXECUTABLE NAMES freetype-config PATHS
     /usr/bin
     /usr/local/bin
     /opt/local/bin
  )

  #reset vars
  set(FREETYPE_LIBRARIES)
  set(FREETYPE_INCLUDE_DIR)

  # if freetype-config has been found
  if(FREETYPECONFIG_EXECUTABLE)

    EXEC_PROGRAM(${FREETYPECONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE FREETYPE_LIBRARIES)

    EXEC_PROGRAM(${FREETYPECONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE FREETYPE_INCLUDE_DIR)
    if(FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)
      set(FREETYPE_FOUND TRUE)
      #message(STATUS "Found freetype: ${FREETYPE_LIBRARIES}")
    endif(FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)

    MARK_AS_ADVANCED(FREETYPE_LIBRARIES FREETYPE_INCLUDE_DIR)

    set( FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES} CACHE INTERNAL "The libraries for freetype" )

  endif(FREETYPECONFIG_EXECUTABLE)

endif (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)
