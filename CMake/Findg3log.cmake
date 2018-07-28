#-----------------------------------------------------------------------------
# Find dependencies
#-----------------------------------------------------------------------------
if(WIN32)
  find_library(DbgHelp_LIBRARY NAMES DbgHelp)
endif()

if(NOT DbgHelp_LIBRARY)
  message(FATAL_ERROR "DbgHelp library not found")
endif()


#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(g3log_INCLUDE_DIR
  NAMES
    g3log/g3log.hpp
    g3log/logworker.hpp
    )
mark_as_advanced(g3log_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(g3log_LIBRARY
  NAMES
    g3logger_shared
    g3logger
  )
mark_as_advanced(g3log_LIBRARY)

set(g3log_LIBRARIES ${g3log_LIBRARY} ${DbgHelp_LIBRARY})

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(g3log
  REQUIRED_VARS
    g3log_INCLUDE_DIR
    g3log_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(G3LOG_FOUND AND NOT TARGET g3log)
  add_library(g3log INTERFACE IMPORTED)
  set_target_properties(g3log PROPERTIES
    INTERFACE_LINK_LIBRARIES "${g3log_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${g3log_INCLUDE_DIR}"
  )
endif()
