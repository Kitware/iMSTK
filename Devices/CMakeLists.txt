
set(EXTERNAL_DEVICES_SOURCES)
set(EXTERNAL_DEVICES_TARGET_LIBRARIES)

if(SIMMEDTK_USE_PHANTOM_OMNI)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    HapticInterface.cpp
    PhantomInterface.cpp
    )
  list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
    OH::HD
    OH::HDU
    )
endif()

if(SIMMEDTK_USE_ADU)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    ADUInterface.cpp)
endif()

if(SIMMEDTK_USE_NIUSB6008)
  list(APPEND EXTERNAL_DEVICES_SOURCES
    NIUSB6008Interface.cpp)
endif()

simmedtk_add_library(Devices
  SOURCES
    DeviceInterface.cpp
    ${EXTERNAL_DEVICES_SOURCES}
  PUBLIC_HEADERS
    ADUInterface.h
    DeviceInterface.h
    HapticInterface.h
    NIDAQmx.h
    Audio.h
    NIUSB6008Interface.h
    PhantomInterface.h
)

target_link_libraries(Devices
  PRIVATE
    Core
  PUBLIC
    ${EXTERNAL_DEVICES_TARGET_LIBRARIES}
)

