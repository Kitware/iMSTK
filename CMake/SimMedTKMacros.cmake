###########################################################################
#
# Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
#                        Rensselaer Polytechnic Institute
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
###########################################################################
#
#  Copyright (c) Kitware Inc.
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

macro (simmedtk_find_package package)
  if (WIN32)
    # TODO: Eventually we won't be needing this
    set(CMAKE_INCLUDE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package_name}"
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package_name}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/lib${package_name}/include"
      )
    set(CMAKE_LIBRARY_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package_name}/lib"
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/lib${package_name}/lib"
      )
  endif (WIN32)

  find_package(${package} ${ARGN})
endmacro ()

function(simmedtk_install_library target)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(target "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  install(TARGETS ${target}
    EXPORT VegaFEMTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
  )
  export(PACKAGE ${target})
  export(TARGETS ${target} ${target_DEPENDS} APPEND FILE ${target}-exports.cmake)
endfunction()

function(simmedtk_add_library target)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES PUBLIC_HEADERS)
  if (NOT WIN32)
    set(libtype SHARED)
  else()
    set(libtype STATIC)
  endif()
  cmake_parse_arguments(target "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  add_library(${target} ${libtype}
    ${target_SOURCES}
  )
  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/simmedtk>
      $<INSTALL_INTERFACE:include/simmedtk/${target}>
    )
  if (target_PUBLIC_HEADERS)
    simmedtk_install_library(${target})
    install(FILES ${target_PUBLIC_HEADERS}
      DESTINATION include/simmedtk/${target}
      COMPONENT Development
    )
  endif()
endfunction()
