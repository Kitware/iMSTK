###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

set(EXTERNAL_DEVICES_SOURCES)
set(EXTERNAL_DEVICES_TARGET_LIBRARIES)

if(USE_VRPN_CLIENT)
    if(USE_VRPN_SERVER)
        list(APPEND EXTERNAL_DEVICES_SOURCES
            VRPNDeviceServer.cpp
        )
        # --------------------------------
        if(iMSTK_USE_PHANTOM_OMNI)
            add_definitions(-DVRPN_USE_PHANTOM_SERVER)
            # --------------------------------
            list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
                OH::HD
                OH::HDU
                OH::HL
                OH::HLU
                vrpn::phantom
                )
        endif(iMSTK_USE_PHANTOM_OMNI)
        # --------------------------------
        if(APPLE)
            find_library(IOKIT_FRAMEWORK IOKit)
            find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
            list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
                ${COREFOUNDATION_FRAMEWORK} ${IOKIT_FRAMEWORK}
            )
        endif(APPLE)
        list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
            vrpn::server
            vrpn::libusb
        )
    endif(USE_VRPN_SERVER)
    # --------------------------------
    list(APPEND EXTERNAL_DEVICES_SOURCES
        VRPNDeviceClient.cpp
        VRPNForceDevice.cpp
        )
    # --------------------------------
    list(APPEND EXTERNAL_DEVICES_TARGET_LIBRARIES
        vrpn::vrpn
        vrpn::quat
        )
endif()

imstk_add_library(Devices
  SOURCES
    DeviceInterface.cpp
    ${EXTERNAL_DEVICES_SOURCES}
  PUBLIC_HEADERS
    DeviceInterface.h
    VRPNDeviceServer.h
    VRPNDeviceClient.h
    VRPNForceDevice.h
)

target_link_libraries(Devices
  PRIVATE
    Core
  PUBLIC
    ${EXTERNAL_DEVICES_TARGET_LIBRARIES}
)
