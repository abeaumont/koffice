
if(LCMS2_FOUND)
  message(STATUS "LCMS version 2 was found, and it will be used.")
  add_subdirectory(lcms2)

else()
  if(HAVE_LCMS)
      message(STATUS "LCMS version 2 was not found, lcms v1 will be used instead.")
      add_subdirectory(lcms)
  endif(HAVE_LCMS)
endif()

