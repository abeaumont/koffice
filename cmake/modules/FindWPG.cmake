# - Try to find LibWpg
# Once done this will define
#
#  LIBWPG_FOUND       - libwpg is available
#  LIBWPG_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBWPG_LIBRARIES   - the libraries needed to use LibWpg
#  LIBWPG_DEFINITIONS - Compiler switches required for using LibWpg
#
# Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.

IF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)

  # Already in cache
  set(LIBWPG_FOUND TRUE)
  
ELSE (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)

  IF (NOT WIN32)
    INCLUDE(FindPkgConfig)
    pkg_check_modules(LIBWPG libwpg-0.2)
    pkg_check_modules(LIBWPS libwps-0.2)
  ENDIF (NOT WIN32)

  FIND_LIBRARY(LIBWPG_STREAM_LIBRARIES NAMES libwps-0.2
	  ${LIBWPG_LIBRARIES}
  )

  FIND_PATH(LIBWPG_INCLUDE_DIR libwpg/libwpg.h
    PATHS
    ${LIBWPG_INCLUDE_DIRS}
    PATH_SUFFIXES libwpg
    )

  FIND_PATH(LIBWPG_STREAM_INCLUDE_DIR libwps/libwps.h
    /usr/include/libwps-0.2
    )

  IF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES AND LIBWPG_STREAM_LIBRARIES AND LIBWPG_STREAM_INCLUDE_DIR)
    SET(LIBWPG_FOUND TRUE)
  ELSE (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES AND LIBWPG_STREAM_LIBRARIES AND LIBWPG_STREAM_INCLUDE_DIR)
    SET(LIBWPG_FOUND FALSE)
  ENDIF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES AND LIBWPG_STREAM_LIBRARIES AND LIBWPG_STREAM_INCLUDE_DIR)
  
  IF (LIBWPG_FOUND)
    MESSAGE(STATUS "Found libwpg: ${LIBWPG_LIBRARIES}")
    MESSAGE("libwpg found " ${LIBWPG_FOUND})
    MESSAGE("libwpg include dir " ${LIBWPG_INCLUDE_DIR})
    MESSAGE("libwpg lib dir " ${LIBWPG_LIBRARY_DIRS})
    MESSAGE("libwpg library " ${LIBWPG_LIBRARIES})
    MESSAGE("libwpg stream include dir " ${LIBWPG_STREAM_INCLUDE_DIR})
    MESSAGE("libwpg stream library " ${LIBWPG_STREAM_LIBRARIES})
    MESSAGE("libwpg cflags " ${LIBWPG_DEFINITIONS})
  ELSE (LIBWPG_FOUND)
    IF (WPG_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find libwpg")
    ENDIF (WPG_FIND_REQUIRED)
  ENDIF (LIBWPG_FOUND)


ENDIF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
