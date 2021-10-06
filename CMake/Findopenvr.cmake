include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for openvr
#-----------------------------------------------------------------------------

imstk_find_header(openvr openvr.h openvr)
imstk_find_libary(openvr openvr_api " ")#Use same library for debug
imstk_find_package(openvr openvr::openvr)

#message(STATUS "openvr include : ${openvr_INCLUDE_DIRS}")
#message(STATUS "openvr libraries : ${openvr_LIBRARIES}")
