include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for openvr
#-----------------------------------------------------------------------------

imstk_find_header(openvr openvr.h)
imstk_find_libary(openvr openvr_api " ")#Use same library for debug
imstk_find_package(openvr)

#message(STATUS "openvr include : ${OPENVR_INCLUDE_DIRS}")
#message(STATUS "openvr libraries : ${OPENVR_LIBRARIES}")
