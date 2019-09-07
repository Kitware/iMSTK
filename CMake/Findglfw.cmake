include(imstkFind)
#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(glfw_INCLUDE_DIR
  NAMES
    GLFW/glfw3.h
  PATH_SUFFIXES
    include
    )
mark_as_advanced(glfw_INCLUDE_DIR)
message(STATUS "glfw_INCLUDE_DIR : ${glfw_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(glfw_LIBRARY
  NAMES
    glfw3
  )
mark_as_advanced(glfw_LIBRARY)
#message(STATUS "glfw_LIBRARY : ${glfw_LIBRARY}")

set(glfw_LIBRARIES ${glfw_LIBRARY})

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glfw
  REQUIRED_VARS
    glfw_INCLUDE_DIR
    glfw_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(GLFW_FOUND AND NOT TARGET glfw)
  add_library(glfw INTERFACE IMPORTED)
  set_target_properties(glfw PROPERTIES
    INTERFACE_LINK_LIBRARIES "${glfw_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${glfw_INCLUDE_DIR}"
  )
endif()
