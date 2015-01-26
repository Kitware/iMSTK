find_path(GLFW_INCLUDE_DIR
  NAMES
    GLFW/glfw3.h
    glfw_config.h
    )

find_library(GLFW_LIBRARY
  NAMES
    glfw
    glfw3
    glfw3dll)

set(GLFW_INCLUDE_DIRS "${GLFW_INCLUDE_DIR}")
set(GLFW_LIBRARIES "${GLFW_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW
  REQUIRED_VARS
    GLFW_INCLUDE_DIR
    GLFW_LIBRARY)

mark_as_advanced(
  GLFW_INCLUDE_DIR
  GLFW_LIBRARY)

if(GLFW_FOUND AND NOT TARGET glfw::glfw)
  add_library(glfw::glfw INTERFACE IMPORTED)
  set_target_properties(glfw::glfw PROPERTIES
    INTERFACE_LINK_LIBRARIES "${GLFW_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}")
endif()
