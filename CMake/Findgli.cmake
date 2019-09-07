include(imstkFind)
#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(gli_INCLUDE_DIR
  NAMES
    gli/gli.hpp
    )
mark_as_advanced(gli_INCLUDE_DIR)
message(STATUS "gli_INCLUDE_DIR : ${gli_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gli
  REQUIRED_VARS
    gli_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(GLM_FOUND AND NOT TARGET gli)
  add_library(gli INTERFACE IMPORTED)
  set_target_properties(gli PROPERTIES
    INTERFACE_LINK_LIBRARIES "${gli_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${gli_INCLUDE_DIR}"
  )
endif()
