#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

list(APPEND Dependencies
  Common
  VRPN)

list(APPEND ExclusionFiles "")
if(${PROJECT_NAME}_USE_OpenHaptics)
  list(APPEND Dependencies
    ${HAPTIC_DEVICE_LIBS})
else()
  list(APPEND ExclusionFiles
    imstkHDAPIDeviceClient.h
    imstkHDAPIDeviceClient.cpp
    imstkHDAPIDeviceServer.h
    imstkHDAPIDeviceServer.cpp)
endif()

include(imstkAddLibrary)
set(HAPTIC_DEVICE_LIBS)
if(${PROJECT_NAME}_USE_OpenHaptics)
  set(HAPTIC_DEVICE_LIBS OpenHapticsSDK)
endif()
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
