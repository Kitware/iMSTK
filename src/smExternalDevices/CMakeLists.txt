
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
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/smADUInterface.h
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/smDeviceInterface.h
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/smHapticInterface.h
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/NIDAQmx.h
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/smAudio.h
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/smNIUSB6008Interface.h
    ${CMAKE_SOURCE_DIR}/include/smExternalDevices/smPhantomInterface.h
)

target_link_libraries(smExternalDevices
  PRIVATE
    smCore
  PUBLIC
    ${EXTERNAL_DEVICES_TARGET_LIBRARIES}
)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(smExternalDevices
    PRIVATE
      -std=c++11
  )
endif()
