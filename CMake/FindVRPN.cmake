#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(VRPN_INCLUDE_DIR
  NAMES
    vrpn_Configure.h
    )
mark_as_advanced(VRPN_INCLUDE_DIR)

set(VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}")
list(APPEND VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}/quat")
list(APPEND VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}/atmellib")
list(APPEND VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}/server_src")
list(APPEND VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}/client_src")
message(STATUS "VRPN_INCLUDE_DIRS : ${VRPN_INCLUDE_DIRS}")

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(VRPN_LIBRARY
  NAMES
    vrpn
    vrpnb
  )
mark_as_advanced(VRPN_LIBRARY)

find_library(QUAT_LIBRARY
  NAMES
    quat
    quatd
  )
mark_as_advanced(QUAT_LIBRARY)

set(VRPN_LIBRARIES ${VRPN_LIBRARY} ${QUAT_LIBRARY})
message(STATUS "VRPN_LIBRARIES : ${VRPN_LIBRARIES}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRPN
  REQUIRED_VARS
    VRPN_INCLUDE_DIRS
    VRPN_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------

if(VRPN_FOUND AND NOT TARGET VRPN)
  add_library(VRPN INTERFACE IMPORTED)
  set_target_properties(VRPN PROPERTIES
    INTERFACE_LINK_LIBRARIES "${VRPN_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${VRPN_INCLUDE_DIRS}"
  )
endif()
