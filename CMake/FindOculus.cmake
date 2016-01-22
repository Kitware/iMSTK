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

find_path(Oculus_INCLUDE_DIR
  NAMES
    OVR.h
    )

find_path(Oculus_SRC_DIR
  NAMES
    OVR_CAPI_GL.h
    )

find_library(Oculus_LIBRARY
  NAMES
    ovr
    ovrd)

set(Oculus_INCLUDE_DIRS "${Oculus_INCLUDE_DIR}" "${Oculus_SRC_DIR}")
set(Oculus_LIBRARIES "${Oculus_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Oculus
  REQUIRED_VARS
    Oculus_INCLUDE_DIR
    Oculus_LIBRARY)

mark_as_advanced(
  Oculus_INCLUDE_DIR
  Oculus_LIBRARY)

if(OCULUS_FOUND AND NOT TARGET Oculus::Oculus)
  add_library(Oculus::Oculus INTERFACE IMPORTED)
  set_target_properties(Oculus::Oculus PROPERTIES
    INTERFACE_LINK_LIBRARIES "${Oculus_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Oculus_INCLUDE_DIR}")
endif()
