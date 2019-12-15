#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
set(HAPTIC_DEVICE_LIBS)
if(${PROJECT_NAME}_USE_OpenHaptics)
  set(HAPTIC_DEVICE_LIBS OpenHapticsSDK)
endif()
imstk_add_library( Devices
  DEPENDS
    Core
    VRPN
    ${HAPTIC_DEVICE_LIBS}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
