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

find_path(ADU_INCLUDE_DIR
  AduHid.h)
find_library(ADU_LIBRARY
  NAMES
    aduhid)

set(ADU_INCLUDE_DIRS "${ADU_INCLUDE_DIR}")
set(ADU_LIBRARIES "${ADU_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ADU
  REQUIRED_VARS
    ADU_INCLUDE_DIR
    ADU_LIBRARY)

mark_as_advanced(
  ADU_INCLUDE_DIR
  ADU_LIBRARY)
