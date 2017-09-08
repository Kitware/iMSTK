#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(glm_INCLUDE_DIR
  NAMES
    glm/glm.hpp
    )
mark_as_advanced(glm_INCLUDE_DIR)
message(STATUS "glm_INCLUDE_DIR : ${glm_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glm
  REQUIRED_VARS
    glm_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(GLM_FOUND AND NOT TARGET glm)
  add_library(glm INTERFACE IMPORTED)
  set_target_properties(glm PROPERTIES
    INTERFACE_LINK_LIBRARIES "${glm_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${glm_INCLUDE_DIR}"
  )
endif()
