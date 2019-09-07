include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for SCCD
#-----------------------------------------------------------------------------

imstk_find_header(SCCD ccdAPI.h)
if(WIN32)
  imstk_find_libary(SCCD sccd)
else()
  imstk_find_libary(SCCD libassimp)
endif()
imstk_find_package(SCCD)

#message(STATUS "SCCD include : ${SCCD_INCLUDE_DIRS}")
#message(STATUS "SCCD libraries : ${SCCD_LIBRARIES}")
