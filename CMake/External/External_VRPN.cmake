
#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------
if(WIN32)
  if(NOT DEFINED VRPN_USE_OpenHaptics)
    set(VRPN_USE_OpenHaptics ${iMSTK_USE_OpenHaptics})
  endif()
else()
  set(VRPN_USE_OpenHaptics OFF)
endif()

#-----------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------
set(VRPN_DEPENDENCIES "LibNiFalcon")
if(WIN32)
  list(APPEND VRPN_DEPENDENCIES "Libusb")
  list(APPEND VRPN_DEPENDENCIES "FTD2XX")
endif(WIN32)
if(VRPN_USE_OpenHaptics)
  list(APPEND VRPN_DEPENDENCIES "OpenHaptics")
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

# Download options
if(NOT DEFINED iMSTK_VRPN_GIT_SHA)
  set(iMSTK_VRPN_GIT_SHA "7a2845e4b1be2707ccb67dd1d388fb22a766e8f7") # vrpn-imstk-additions
endif()
if(NOT DEFINED iMSTK_VRPN_GIT_REPOSITORY)
  set(iMSTK_VRPN_GIT_REPOSITORY "https://gitlab.kitware.com/iMSTK/vrpn.git")
endif()

include(imstkAddExternalProject)
imstk_add_external_project( VRPN
  GIT_REPOSITORY ${iMSTK_VRPN_GIT_REPOSITORY}
  GIT_TAG ${iMSTK_VRPN_GIT_SHA}
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
    -DVRPN_USE_PHANTOM_SERVER:BOOL=${VRPN_USE_OPENHAPTICS}
    -DVRPN_USE_HDAPI:BOOL=${VRPN_USE_OPENHAPTICS}
    -DOPENHAPTICS_ROOT_DIR:PATH=${OPENHAPTICS_ROOT_DIR}
  DEPENDENCIES ${VRPN_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)

ExternalProject_Message("VRPN" "VRPN: Phantom Omni support [${VRPN_USE_OpenHaptics}]")
