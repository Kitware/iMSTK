find_path(VRPN_INCLUDE_DIR
  NAMES
    vrpn_Configure.h
    )

find_library(VRPN_LIBRARY
  NAMES
    vrpn
    vrpnd)

find_library(VRPN_QUAT_LIBRARY
  NAMES
    quat
    quatd)

set(VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}")
set(VRPN_LIBRARIES "${VRPN_LIBRARY} ${VRPN_QUAT_LIBRARY}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(VRPN
  REQUIRED_VARS
    VRPN_INCLUDE_DIR
    VRPN_LIBRARY
    VRPN_QUAT_LIBRARY)

mark_as_advanced(
  VRPN_INCLUDE_DIR
  VRPN_LIBRARY
  VRPN_QUAT_LIBRARY)

if(VRPN_FOUND AND NOT TARGET vrpn::vrpn)
  add_library(vrpn::vrpn INTERFACE IMPORTED)
  set_target_properties(vrpn::vrpn PROPERTIES
    INTERFACE_LINK_LIBRARIES "${VRPN_LIBRARY};${VRPN_QUAT_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${VRPN_INCLUDE_DIR}")
endif()
