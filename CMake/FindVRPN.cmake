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

find_path(VRPN_INCLUDE_DIR
  NAMES
    vrpn_Configure.h
    )

find_library(VRPN_LIBRARY
  NAMES
    vrpn
    vrpnd)

set(VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIR}")
set(VRPN_LIBRARIES "${VRPN_LIBRARY}")

# Macro used to create link to target library
macro(CREATE_VRPN_TARGET_LINK TARGET_NAME TARGET_LIBRARY TARGET_INCLUDES)
  if(NOT TARGET ${TARGET_NAME})
    add_library(${TARGET_NAME} INTERFACE IMPORTED)
    set_target_properties(${TARGET_NAME} PROPERTIES
        INTERFACE_LINK_LIBRARIES "${TARGET_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${TARGET_INCLUDES}")
  endif()
endmacro()

set(VRPN_REQUIRED_VARS)
if(VRPN_FIND_COMPONENTS)
  # Treat dependencies as components
  foreach(component ${VRPN_FIND_COMPONENTS})
    string(TOUPPER ${component} _COMPONENT)
    if(${_COMPONENT} STREQUAL "LIBNIFALCON")
      find_package(${component} QUIET)
      list(APPEND VRPN_INCLUDE_DIRS "${${_COMPONENT}_INCLUDE_DIR}")
      list(APPEND VRPN_REQUIRED_VARS ${_COMPONENT}_LIBRARY ${_COMPONENT}_INCLUDE_DIR)
    elseif(${_COMPONENT} STREQUAL "LIBUSB1")
      find_package(${component})
      list(APPEND VRPN_INCLUDE_DIRS "${${_COMPONENT}_INCLUDE_DIR}")
      list(APPEND VRPN_REQUIRED_VARS ${_COMPONENT}_LIBRARY ${_COMPONENT}_INCLUDE_DIR)
    else()
      find_library(${_COMPONENT}_LIBRARY
        NAMES
          ${component}
          ${component}d)
    endif()
    list(APPEND VRPN_LIBRARIES "${${_COMPONENT}_LIBRARY}")
    list(APPEND VRPN_REQUIRED_VARS
      ${_COMPONENT}_LIBRARY)

  endforeach()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(VRPN
  REQUIRED_VARS
    VRPN_INCLUDE_DIR
    VRPN_LIBRARY
    ${VRPN_REQUIRED_VARS})

mark_as_advanced(
  VRPN_INCLUDE_DIR
  VRPN_LIBRARY
  ${VRPN_REQUIRED_VARS})

if(VRPN_FOUND)
  create_vrpn_target_link(vrpn::vrpn ${VRPN_LIBRARY} ${VRPN_INCLUDE_DIR})
  if(VRPN_FIND_COMPONENTS)
    foreach(component ${VRPN_FIND_COMPONENTS})
      string(TOUPPER ${component} _COMPONENT)
      set(_target)
      if(${_COMPONENT} STREQUAL "LIBNIFALCON")
        set(_target vrpn::libfalcon)
      elseif(${_COMPONENT} STREQUAL "LIBUSB1")
        set(_target vrpn::libusb)
      elseif(${_COMPONENT} STREQUAL "QUAT")
        set(_target vrpn::${component})
        set(${_COMPONENT}_INCLUDE_DIR "${VRPN_INCLUDE_DIR}/quat")
      elseif(${_COMPONENT} STREQUAL "VRPNSERVER")
        set(_target vrpn::server)
        set(${_COMPONENT}_INCLUDE_DIR "${VRPN_INCLUDE_DIR}/server_src")
      elseif(${_COMPONENT} STREQUAL "VRPN_PHANTOM")
        set(_target vrpn::phantom)
        set(${_COMPONENT}_INCLUDE_DIR "${VRPN_INCLUDE_DIR}/server_src")
      else()
        set(_target vrpn::${component})
      endif()
      create_vrpn_target_link(${_target} "${${_COMPONENT}_LIBRARY}" "${${_COMPONENT}_INCLUDE_DIR}")
    endforeach()
  endif()
endif()
