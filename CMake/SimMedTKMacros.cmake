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
    if("${package}" STREQUAL "Eigen3")
      set(package_name "Eigen")
    else()
      set(package_name ${package})
    endif()

    # TODO: Set the right variables.
    # TODO: Make this work in Visual Studio and switch between release and debug properly.
    set(CMAKE_INCLUDE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package_name}"
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package_name}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/lib${package_name}/include"
      )
#    message("${CMAKE_INCLUDE_PATH}")
    set(CMAKE_LIBRARY_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package_name}/lib"
      "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/lib${package_name}/lib"
      )
#    message("${CMAKE_LIBRARY_PATH}")
    if("${package}" STREQUAL "Eigen")
      set(package Eigen3)
    endif()
  endif (WIN32)

  find_package(${package} ${ARGN})
endmacro ()
