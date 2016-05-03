#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( VRPN
  REPOSITORY https://github.com/vrpn/vrpn.git
  GIT_TAG bc1788410478f4fbedab479524537f2524ca9262
  CMAKE_ARGS
    -DBUILD_TESTING:BOOL=OFF
    -DVRPN_SUBPROJECT_BUILD:BOOL=ON
    -DVRPN_BUILD_CLIENTS:BOOL=OFF
    -DVRPN_BUILD_SERVERS:BOOL=OFF
    -DVRPN_BUILD_CLIENT_LIBRARY:BOOL=ON
    -DVRPN_BUILD_SERVER_LIBRARY:BOOL=ON
    -DVRPN_INSTALL:BOOL=OFF
    -DVRPN_BUILD_PYTHON:BOOL=OFF
    -DVRPN_USE_GPM_MOUSE:BOOL=OFF
    -DVRPN_USE_HID:BOOL=ON
    #-DVRPN_USE_LIBNIFALCON:BOOL=ON
  #DEPENDENCIES "LibNiFalcon"
  #RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
