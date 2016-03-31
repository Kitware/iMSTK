#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(PTHREAD_INCLUDE_DIR
  NAMES
    PThreads.h
  HINTS
    ${PThreads_SOURCE_DIR}/include
    )
mark_as_advanced(PTHREAD_INCLUDE_DIR)
#message(STATUS "PTHREAD_INCLUDE_DIR : ${PTHREAD_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(PTHREAD_LIBRARY
  NAMES
	PThreads
	libPThreads
  HINTS
    ${PThreads_DIR}
  )
mark_as_advanced(PTHREAD_LIBRARY)
#message(STATUS "PTHREAD_LIBRARY : ${PTHREAD_LIBRARY}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PTHREAD
  REQUIRED_VARS
    PTHREAD_INCLUDE_DIR
    PTHREAD_LIBRARY)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(PThreads_FOUND AND NOT TARGET Threads::Threads)
  add_library(Threads::Threads INTERFACE IMPORTED)
  set_target_properties(Threads::Threads PROPERTIES
    INTERFACE_LINK_LIBRARIES "${PTHREAD_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PTHREAD_INCLUDE_DIR}"
  )
endif()
