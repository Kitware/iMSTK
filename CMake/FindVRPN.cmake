# Comments by Alexis:
# VRPN has a FindVRPN, Findquatlib, FindLibusb1, FindHIDAPI in cmake folder:
# - FindLibusb1 finds the include but not the lib. Updating it from usb-1.0 to
#   libusb-1.0 fixes the issue.
# - Findquatlib does not find lib or include. Updating lib from quat.lib/libquat.a
#   to quat and quatd resolved the library. No way to resolve the include since
#   QUATLIB_ROOT_DIR is not defined in this scope.
# - FindHIDAPI does not find lib or include. Using submodules/hidapi.cmake
#   instead would help finding the good variables but requires more includes and
#   to be in VRPN scope.
# - FindVRPN uses Findquatlib and FindLibusb1 therefore fails. It also does not
#   look for HIDAPI.

#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(VRPN_INCLUDE_DIR
  NAMES
    vrpn_Configure.h
    )
mark_as_advanced(VRPN_INCLUDE_DIR)
list(APPEND VRPN_INCLUDE_DIRS
  ${VRPN_INCLUDE_DIR}
  ${VRPN_INCLUDE_DIR}/quat
  ${VRPN_INCLUDE_DIR}/atmellib
  )

find_path(LIBNIFALCON_INCLUDE_DIR
  NAMES
    falcon/core/FalconDevice.h
  )
mark_as_advanced(LIBNIFALCON_INCLUDE_DIR)
list(APPEND VRPN_INCLUDE_DIRS ${LIBNIFALCON_INCLUDE_DIR})

find_path(LIBUSB1_INCLUDE_DIR
  NAMES
    libusb.h
    libusb-1.0/libusb.h
  )
mark_as_advanced(LIBUSB1_INCLUDE_DIR)
list(APPEND VRPN_INCLUDE_DIRS ${LIBUSB1_INCLUDE_DIR})

if(WIN32)
  find_path(LIBFTD2XX_INCLUDE_DIR
    NAMES
      ftd2xx.h
    )
  mark_as_advanced(LIBFTD2XX_INCLUDE_DIR)
  list(APPEND VRPN_INCLUDE_DIRS ${LIBFTD2XX_INCLUDE_DIR})
endif()

#-----------------------------------------------------------------------------
# Phantom Omni support
#-----------------------------------------------------------------------------
if(${iMSTK_USE_OMNI})

  find_library(VRPN_PHANTOM_LIBRARY
    NAMES
      vrpn_phantom
      vrpn_phantomd
    )
  mark_as_advanced(VRPN_PHANTOM_LIBRARY)
  list(APPEND VRPN_LIBRARIES ${VRPN_PHANTOM_LIBRARY})

  list(APPEND CMAKE_MODULE_PATH ${VRPN_INCLUDE_DIR}/cmake)
  find_package(OpenHaptics)
  list(APPEND VRPN_LIBRARIES ${OPENHAPTICS_LIBRARIES})
  list(APPEND VRPN_INCLUDE_DIRS ${OPENHAPTICS_INCLUDE_DIRS})
  list(REMOVE_ITEM CMAKE_MODULE_PATH ${VRPN_INCLUDE_DIR}/cmake)

  find_package(OpenGL)
  list(APPEND VRPN_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND VRPN_LIBRARIES ${OPENGL_LIBRARY})

endif()

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------

find_library(VRPN_LIBRARY
  NAMES
    vrpn
    vrpnd
  )
mark_as_advanced(VRPN_LIBRARY)
list(APPEND VRPN_LIBRARIES ${VRPN_LIBRARY})

find_library(VRPNSERVER_LIBRARY
  NAMES
    vrpnserver
    vrpnserverd
  )
mark_as_advanced(VRPNSERVER_LIBRARY)
list(APPEND VRPN_LIBRARIES ${VRPNSERVER_LIBRARY})

find_library(QUAT_LIBRARY
  NAMES
    quat
    quatd
  )
mark_as_advanced(QUAT_LIBRARY)
list(APPEND VRPN_LIBRARIES ${QUAT_LIBRARY})

find_library(LIBNIFALCON_LIBRARY
  NAMES
    libnifalcon
    nifalcon
  )
mark_as_advanced(LIBNIFALCON_LIBRARY)
list(APPEND VRPN_LIBRARIES ${LIBNIFALCON_LIBRARY})

#works on windows, but sounds like it is needed only on linux, check vrpn/submodules/hidapi.cmake
find_library(LIBUSB1_LIBRARY
  NAMES
    libusb-1.0
    usb-1.0
  )
mark_as_advanced(LIBUSB1_LIBRARY)
list(APPEND VRPN_LIBRARIES ${LIBUSB1_LIBRARY})

if(WIN32)
  find_library(LIBFTD2XX_LIBRARY
    NAMES
      ftd2xx
      libftd2xx
    )
  mark_as_advanced(LIBFTD2XX_LIBRARY)
  list(APPEND VRPN_LIBRARIES ${LIBFTD2XX_LIBRARY})
endif()

#check vrpn/submodules/hidapi.cmake
if(WIN32)
  find_library(HIDAPI_LIBRARY
    NAMES
      setupapi
    )
elseif(APPLE)
  find_library(MACHID_CoreFoundation_LIBRARY CoreFoundation)
  find_library(MACHID_IOKit_LIBRARY IOKit)
  set(HIDAPI_LIBRARY ${MACHID_CoreFoundation_LIBRARY} ${MACHID_IOKit_LIBRARY})
endif()
list(APPEND VRPN_LIBRARIES ${HIDAPI_LIBRARY})
mark_as_advanced(HIDAPI_LIBRARY)

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
