include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for openvr
#-----------------------------------------------------------------------------

set(_sub_dir "")
if(NOT DEFINED OpenVR_ROOT_DIR)
  set(_sub_dir "OpenVR")
endif()

imstk_find_header(OpenVR openvr.h ${_sub_dir})
imstk_find_libary(OpenVR openvr_api " ")#Use same library for debug
imstk_find_package(OpenVR OpenVR::OpenVR)

#message(STATUS "OpenVR include : ${OpenVR_INCLUDE_DIRS}")
#message(STATUS "OpenVR libraries : ${OpenVR_LIBRARIES}")
