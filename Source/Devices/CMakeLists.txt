#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

list(APPEND Dependencies Common Scene SceneEntities)

set(H_FILES
  imstkDeviceClient.h
  imstkDeviceManagerFactory.h
  imstkDeviceManager.h
  imstkDummyClient.h
  imstkKeyboardDeviceClient.h
  imstkMouseDeviceClient.h
  imstkProgrammableClient.h
  imstkOpenVRDeviceClient.h
  )

set(SRC_FILES
  imstkDeviceClient.cpp
  imstkDeviceManagerFactory.cpp
  imstkDummyClient.cpp
  imstkKeyboardDeviceClient.cpp
  imstkMouseDeviceClient.cpp
  imstkProgrammableClient.cpp
  imstkOpenVRDeviceClient.cpp
  )

if(iMSTK_USE_OpenHaptics)
  list(APPEND Dependencies OpenHapticsSDK::OpenHapticsSDK)

  list(APPEND H_FILES
    imstkOpenHapticDeviceClient.h
    imstkOpenHapticDeviceManager.h
    )
  list(APPEND SRC_FILES
    imstkOpenHapticDeviceClient.cpp
    imstkOpenHapticDeviceManager.cpp
    )
endif()

if (iMSTK_USE_HAPLY)
  list(APPEND Dependencies Haply::Haply)

  list(APPEND H_FILES
    imstkHaplyDeviceClient.h
    imstkHaplyDeviceManager.h
    )
  list(APPEND SRC_FILES
    imstkHaplyDeviceClient.cpp
    imstkHaplyDeviceManager.cpp
    )
endif()

if(iMSTK_USE_VRPN)
  list(APPEND Dependencies VRPN::VRPN)

  list(APPEND H_FILES
    imstkVRPNDeviceClient.h
    imstkVRPNDeviceManager.h
    )
  list(APPEND SRC_FILES
    imstkVRPNDeviceClient.cpp
    imstkVRPNDeviceManager.cpp
    )
endif()

include(imstkAddLibrary)
imstk_add_library(Devices
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    ${Dependencies}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
