
set(EXTERNAL_DEVICES_SOURCES)
set(EXTERNAL_DEVICES_TARGET_LIBRARIES)

if(SIMMEDTK_USE_PHANTOM_OMNI)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    smHapticInterface.cpp
    smPhantomInterface.cpp
    )
  list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
    OH::HD
    OH::HDU
    )
endif()

if(SIMMEDTK_USE_ADU)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    smADUInterface.cpp)
endif()

if(SIMMEDTK_USE_NIUSB6008)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    smNIUSB6008Interface.cpp)
endif()

simmedtk_add_library(smExternalDevices
  SOURCES
    smDeviceInterface.cpp
    ${EXTERNAL_DEVICES_SOURCES}
  PUBLIC_HEADERS
    smADUInterface.h
    smDeviceInterface.h
    smHapticInterface.h
    NIDAQmx.h
    smAudio.h
    smNIUSB6008Interface.h
    smPhantomInterface.h
)

target_link_libraries(smExternalDevices
  PRIVATE
    smCore
  PUBLIC
    ${EXTERNAL_DEVICES_TARGET_LIBRARIES}
)

