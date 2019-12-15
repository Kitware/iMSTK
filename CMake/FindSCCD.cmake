include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for SCCD
#-----------------------------------------------------------------------------

imstk_find_header(SCCD ccdAPI.h sccd)
imstk_find_libary(SCCD sccd)
imstk_find_package(SCCD)

#message(STATUS "SCCD include : ${SCCD_INCLUDE_DIRS}")
#message(STATUS "SCCD libraries : ${SCCD_LIBRARIES}")
