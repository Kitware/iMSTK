#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

list(APPEND Dependencies Common)

if(iMSTK_USE_OpenHaptics)
  list(APPEND Dependencies OpenHapticsSDK)
endif()
if(iMSTK_USE_VRPN)
  list(APPEND Dependencies VRPN)
endif()

list(APPEND ExclusionFiles "")
if(NOT iMSTK_USE_OpenHaptics)
  list(APPEND ExclusionFiles
    imstkHapticDeviceClient.h
    imstkHapticDeviceClient.cpp
    imstkHapticDeviceManager.h
    imstkHapticDeviceManager.cpp)
endif()
if(NOT iMSTK_USE_VRPN)
  list(APPEND ExclusionFiles
    imstkVRPNDeviceManager.h
    imstkVRPNDeviceManager.cpp
    imstkVRPNDeviceClient.h
    imstkVRPNDeviceClient.cpp)
endif()

include(imstkAddLibrary)
imstk_add_library(Devices
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    ${Dependencies})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
