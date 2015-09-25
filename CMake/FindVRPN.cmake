find_path(VRPN_INCLUDE_DIR
  NAMES
    vrpn_Configure.h
    )

find_library(VRPN_LIBRARY
  NAMES
    vrpn
    vrpnd)

find_library(VRPN_SERVER_LIBRARY
  NAMES
    vrpnserver
    vrpnserverd)

find_library(VRPN_PHANTOM_LIBRARY
  NAMES
    vrpn_phantom
    vrpn_phantomd)

find_library(VRPN_QUAT_LIBRARY
  NAMES
    quat
    quatd)

set(VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}")
set(VRPN_LIBRARIES "${VRPN_LIBRARY} ${VRPN_QUAT_LIBRARY} ${VRPN_SERVER_LIBRARY} ${VRPN_PHANTOM_LIBRARY}")

# Try to find libusb dependency
find_package(Libusb1 QUIET)
if(LIBUSB1_FOUND)
    list(APPEND VRPN_LIBRARIES ${LIBUSB1_LIBRARIES})
    list(APPEND VRPN_INCLUDE_DIRS ${LIBUSB1_INCLUDE_DIRS})
endif()

# Try to find falcon dependency
find_package(LibNifalcon QUIET)
if(LIBUSB1_FOUND)
    list(APPEND VRPN_LIBRARIES ${LIBNIFALCON_LIBRARIES})
    list(APPEND VRPN_INCLUDE_DIRS ${LIBNIFALCON_INCLUDE_DIRS})
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(VRPN
  REQUIRED_VARS
    VRPN_INCLUDE_DIR
    VRPN_LIBRARY
    VRPN_QUAT_LIBRARY
    VRPN_SERVER_LIBRARY
    VRPN_PHANTOM_LIBRARY)

mark_as_advanced(
  VRPN_INCLUDE_DIR
  VRPN_LIBRARY
  VRPN_QUAT_LIBRARY
  VRPN_SERVER_LIBRARY
  VRPN_PHANTOM_LIBRARY)

if(VRPN_FOUND)
  if(NOT TARGET vrpn::vrpn)
    add_library(vrpn::vrpn INTERFACE IMPORTED)
    set_target_properties(vrpn::vrpn PROPERTIES
        INTERFACE_LINK_LIBRARIES "${VRPN_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${VRPN_INCLUDE_DIR}")
  endif()
  if(LIBUSB1_FOUND)
    if(NOT TARGET vrpn::libusb)
        add_library(vrpn::libusb INTERFACE IMPORTED)
        set_target_properties(vrpn::libusb PROPERTIES
            INTERFACE_LINK_LIBRARIES "${LIBUSB1_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBUSB1_INCLUDE_DIRS}")
    endif()
  endif(LIBUSB1_FOUND)
  if(LIBNIFALCON_FOUND)
    if(NOT TARGET vrpn::libfalcon)
        add_library(vrpn::libfalcon INTERFACE IMPORTED)
        set_target_properties(vrpn::libfalcon PROPERTIES
            INTERFACE_LINK_LIBRARIES "${LIBNIFALCON_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBNIFALCON_INCLUDE_DIRS}")
    endif()
  endif(LIBNIFALCON_FOUND)
  if(NOT TARGET vrpn::quat)
    add_library(vrpn::quat INTERFACE IMPORTED)
    set_target_properties(vrpn::quat PROPERTIES
        INTERFACE_LINK_LIBRARIES "${VRPN_QUAT_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${VRPN_INCLUDE_DIR}/quat")
  endif()

  if(NOT TARGET vrpn::server)
    add_library(vrpn::server INTERFACE IMPORTED)
    set_target_properties(vrpn::server PROPERTIES
        INTERFACE_LINK_LIBRARIES "${VRPN_SERVER_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${VRPN_INCLUDE_DIR}/server_src")
  endif()

  if(NOT TARGET vrpn::phantom)
    add_library(vrpn::phantom INTERFACE IMPORTED)
    set_target_properties(vrpn::phantom PROPERTIES
        INTERFACE_LINK_LIBRARIES "${VRPN_PHANTOM_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${VRPN_INCLUDE_DIR}/server_src")
  endif()

endif()
