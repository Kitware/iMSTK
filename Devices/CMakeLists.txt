
set(EXTERNAL_DEVICES_SOURCES)
set(EXTERNAL_DEVICES_TARGET_LIBRARIES)

if(USE_VRPN_CLIENT OR USE_VRPN_SERVER)
  list(APPEND EXTERNAL_DEVICES_SOURCES
#     HapticInterface.cpp
#    PhantomInterface.cpp
    VRPNPhantomDevice.cpp
    )
  list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
    vrpn::vrpn
    vrpn::quat
    )
endif()

if(SimMedTK_USE_ADU)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    ADUInterface.cpp)
endif()

if(SimMedTK_USE_NIUSB6008)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    NIUSB6008Interface.cpp)
endif()

simmedtk_add_library(Devices
  SOURCES
    DeviceInterface.cpp
    ${EXTERNAL_DEVICES_SOURCES}
  PUBLIC_HEADERS
#     ADUInterface.h
#     HapticInterface.h
#     NIDAQmx.h
#     Audio.h
#     NIUSB6008Interface.h
#     PhantomInterface.h
    DeviceInterface.h
    VRPNPhantomDevice.h
)

target_link_libraries(Devices
  PRIVATE
    Core
  PUBLIC
    ${EXTERNAL_DEVICES_TARGET_LIBRARIES}
)

