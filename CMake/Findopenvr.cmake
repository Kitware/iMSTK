#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(openvr_INCLUDE_DIR
  NAMES
    openvr.h
    )
mark_as_advanced(openvr_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(openvr_LIBRARIES
  NAMES
    openvr_api
  )
mark_as_advanced(openvr_LIBRARIES)
message(STATUS "openvr_INCLUDE_DIR : ${openvr_LIBRARIES}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(openvr
  REQUIRED_VARS
    openvr_INCLUDE_DIR
    openvr_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(openvr_FOUND AND NOT TARGET openvr)
  add_library(openvr INTERFACE IMPORTED)
  set_target_properties(openvr PROPERTIES
    INTERFACE_LINK_LIBRARIES "${openvr_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${openvr_INCLUDE_DIR}"
  )
endif()
