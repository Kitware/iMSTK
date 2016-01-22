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

find_path(ThreadPool_INCLUDE_DIR
  NAMES
    ThreadPool.h
    )

set(ThreadPool_INCLUDE_DIRS "${ThreadPool_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ThreadPool
  REQUIRED_VARS
    ThreadPool_INCLUDE_DIR)

mark_as_advanced(
  ThreadPool_INCLUDE_DIR)
