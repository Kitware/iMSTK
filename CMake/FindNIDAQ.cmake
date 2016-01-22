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

find_path(NIDAQ_INCLUDE_DIR
  NIDAQmx.h)
find_library(NIDAQ_mx_LIBRARY
  NAMES
    NIDAQmx)
find_library(NIDAQ_syscfg_LIBRARY
  NAMES
    nisyscfg)

set(NIDAQ_INCLUDE_DIRS "${NIDAQ_INCLUDE_DIR}")
set(NIDAQ_LIBRARIES "${NIDAQ_mx_LIBRARY}" "${NIDQA_syscfg_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NIDAQ
  REQUIRED_VARS
    NIDAQ_INCLUDE_DIR
    NIDAQ_mx_LIBRARY
    NIDAQ_syscfg_LIBRARY)

mark_as_advanced(
  NIDAQ_INCLUDE_DIR
  NIDAQ_mx_LIBRARY
  NIDAQ_syscfg_LIBRARY)
