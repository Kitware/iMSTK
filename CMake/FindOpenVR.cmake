include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for openvr
#-----------------------------------------------------------------------------

imstk_find_header(OpenVR openvr.h openvr)
imstk_find_libary(OpenVR openvr_api " ")#Use same library for debug
imstk_find_package(OpenVR OpenVR::OpenVR)

#message(STATUS "OpenVR include : ${OpenVR_INCLUDE_DIRS}")
#message(STATUS "OpenVR libraries : ${OpenVR_LIBRARIES}")
