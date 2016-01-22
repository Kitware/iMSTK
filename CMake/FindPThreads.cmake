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

if(WIN32)
  find_path(PTHREAD_INCLUDE_DIR
      pthread.h
      )

  find_library(PTHREAD_LIBRARY
    NAMES
      pthread
      libpthread
      )

  set(PTHREAD_INCLUDE_DIRS "${PTHREAD_INCLUDE_DIR}")
  set(PTHREAD_LIBRARIES "${PTHREAD_LIBRARY}")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PTHREAD
    REQUIRED_VARS
      PTHREAD_INCLUDE_DIR
      PTHREAD_LIBRARY)

  mark_as_advanced(
    PTHREAD_INCLUDE_DIR
    PTHREAD_LIBRARY)

  if(PTHREAD_FOUND AND NOT TARGET Threads::Threads)
    add_library(Threads::Threads INTERFACE IMPORTED)
    set_target_properties(Threads::Threads PROPERTIES
      INTERFACE_LINK_LIBRARIES "${PTHREAD_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${PTHREAD_INCLUDE_DIRS}")
  endif()
else()
  set(required)
  if(PThreads_FIND_REQUIRED)
    set(required REQUIRED)
  endif()
  find_package(Threads ${required})
endif()
