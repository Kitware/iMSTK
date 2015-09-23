
set(EXTERNAL_DEVICES_SOURCES)
set(EXTERNAL_DEVICES_TARGET_LIBRARIES)

if(USE_VRPN_CLIENT)
    if(USE_VRPN_SERVER)
        list(APPEND EXTERNAL_DEVICES_SOURCES
            VRPNDeviceServer.cpp
        )
        # --------------------------------
        if(SimMedTK_USE_PHANTOM_OMNI)
            add_definitions(-DVRPN_USE_PHANTOM_SERVER)
            # --------------------------------
            list(APPEND EXTERNAL_DEVICES_SOURCES
                VRPNPhantomDevice.cpp
                )
            # --------------------------------
            list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
                OH::HD
                OH::HDU
                OH::HL
                OH::HLU
                vrpn::phantom
                )
        endif()
        # --------------------------------
        list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
            vrpn::server
        )
    endif(USE_VRPN_SERVER)
    # --------------------------------
    list(APPEND EXTERNAL_DEVICES_SOURCES
        VRPNDeviceClient.cpp
        )
    # --------------------------------
    list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
        vrpn::vrpn
        vrpn::quat
        vrpn::libusb
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
    DeviceInterface.h
    VRPNDeviceServer.h
    VRPNDeviceClient.h
    VRPNPhantomDevice.h
)

target_link_libraries(Devices
  PRIVATE
    Core
  PUBLIC
    ${EXTERNAL_DEVICES_TARGET_LIBRARIES}
)

