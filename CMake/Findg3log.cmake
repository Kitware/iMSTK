#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(g3log_INCLUDE_DIR
  NAMES
    g3log/g3log.hpp
    g3log/logworker.hpp
  HINTS
    ${g3log_SOURCE_DIR}/src
    )
mark_as_advanced(g3log_INCLUDE_DIR)
#message(STATUS "g3log_INCLUDE_DIR : ${g3log_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(g3log_LIBRARY
  NAMES
    g3logger_shared
  HINTS
    ${g3log_DIR}
  )
mark_as_advanced(g3log_LIBRARY)
#message(STATUS "g3log_LIBRARY : ${g3log_LIBRARY}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(g3log
  REQUIRED_VARS
    g3log_INCLUDE_DIR
    g3log_LIBRARY)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(G3LOG_FOUND AND NOT TARGET g3log)
  add_library(g3log INTERFACE IMPORTED)
  set_target_properties(g3log PROPERTIES
    INTERFACE_LINK_LIBRARIES "${g3log_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${g3log_INCLUDE_DIR}"
  )
endif()
