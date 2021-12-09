#-----------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------
set(VRPN_DEPENDENCIES "LibNiFalcon")
if(WIN32)
  list(APPEND VRPN_DEPENDENCIES "Libusb")
  list(APPEND VRPN_DEPENDENCIES "FTD2XX")
endif(WIN32)

#-----------------------------------------------------------------------------
# Phantom Omni
#-----------------------------------------------------------------------------
option(${PROJECT_NAME}_USE_OpenHaptics "Build OpenHaptics to support the Phantom Omni in VRPN." OFF)
if(${${PROJECT_NAME}_USE_OpenHaptics})
  message(STATUS "Superbuild -   VRPN => ENABLING Phantom Omni support")
  if(NOT DEFINED OPENHAPTICS_ROOT_DIR OR NOT EXISTS ${OPENHAPTICS_ROOT_DIR})
    set(OPENHAPTICS_ROOT_DIR "$ENV{OH_SDK_BASE}" CACHE PATH "Path to OpenHaptics install directory." FORCE)
  endif()
  if(NOT EXISTS ${OPENHAPTICS_ROOT_DIR})
    message(FATAL_ERROR "\nCan not support Phantom Omni without OpenHaptics.\nSet OPENHAPTICS_ROOT_DIR to OpenHaptics installation directory.\n\n")
  endif()
  list(APPEND VRPN_DEPENDENCIES "OpenHaptics")
else()
  message(STATUS "Superbuild -   VRPN => Phantom Omni support DISABLED")
  if(DEFINED OPENHAPTICS_ROOT_DIR)
    unset(OPENHAPTICS_ROOT_DIR CACHE)
  endif()

endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( VRPN
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/vrpn.git
  GIT_TAG 7a2845e4b1be2707ccb67dd1d388fb22a766e8f7 # vrpn-imstk-additions
  # Cannot get a zip as vrpn uses submodules which are not pulled into the zip
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
    -DVRPN_SUBPROJECT_BUILD:BOOL=ON
    -DVRPN_BUILD_CLIENTS:BOOL=OFF
    -DVRPN_BUILD_CLIENT_LIBRARY:BOOL=ON
    -DVRPN_BUILD_SERVER_LIBRARY:BOOL=ON
    -DVRPN_INSTALL:BOOL=ON
    -DVRPN_BUILD_PYTHON:BOOL=OFF
    -DVRPN_USE_GPM_MOUSE:BOOL=OFF
    -DVRPN_USE_LIBUSB_1_0:BOOL=ON
    -DVRPN_USE_HID:BOOL=ON
    -DVRPN_USE_LIBNIFALCON:BOOL=OFF
    -DVRPN_BUILD_SERVERS:BOOL=ON
    -DVRPN_USE_PHANTOM_SERVER:BOOL=${iMSTK_USE_OPENHAPTICS}
    -DVRPN_USE_HDAPI:BOOL=${iMSTK_USE_OPENHAPTICS}
    -DOPENHAPTICS_ROOT_DIR:PATH=${OPENHAPTICS_ROOT_DIR}
  DEPENDENCIES ${VRPN_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)