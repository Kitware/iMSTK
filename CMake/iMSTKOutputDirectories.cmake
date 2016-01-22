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

#-----------------------------------------------------------------------------
# Output directories - this is where built library and executable
# files will be placed after building but prior to install.  The
# necessary variables change between single and multi configuration
# build systems, so it is necessary to handle both cases on a
# conditional basis.
#
set(RUNTIME_DIRNAME bin)
set(LIBRARY_DIRNAME lib)
set(ARCHIVE_DIRNAME lib)
if(WIN32)
  set(LIBRARY_DIRNAME bin)
  set(ARCHIVE_DIRNAME bin)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
  foreach(type LIBRARY RUNTIME ARCHIVE)
    # Make sure the directory exists
    if(DEFINED iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY
        AND NOT EXISTS ${iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY})
      message(FATAL_ERROR "iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY is set to a non-existing directory [${iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY}]")
    endif()

    if(iMSTK_SUPERBUILD)
      set(output_dir ${iMSTK_BINARY_DIR}/${${type}_DIRNAME})
      if(NOT DEFINED iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY)
        set(iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY ${iMSTK_BINARY_DIR}/iMSTK-build/${${type}_DIRNAME})
      endif()
    else()
      if(NOT DEFINED iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY)
        set(output_dir ${iMSTK_BINARY_DIR}/${${type}_DIRNAME})
      else()
        set(output_dir ${iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY})
      endif()
    endif()
    set(CMAKE_${type}_OUTPUT_DIRECTORY ${output_dir} CACHE INTERNAL "Single output directory for building all libraries.")
  endforeach()
else()
  # Multi-configuration is more difficult.  Not only do we need to
  # properly set the output directories, but we also need to
  # identify the "toplevel" directory for each configuration so
  # we can place files, documentation, etc. in the correct
  # relative positions.  Because files may be placed by CMake
  # without a build target to put them in their proper relative build
  # directory position using these paths, we must fully qualify them
  # without using CMAKE_CFG_INTDIR.
  #
  # We define directories that may not be quite "standard"
  # for a particular build tool - for example, native VS2010 projects use
  # another directory to denote CPU type being compiled for - but CMake only
  # supports multi-configuration setups having multiple configurations,
  # not multiple compilers.
  #
  # One additional wrinkle we must watch for here is the case where
  # a multi-configuration setup uses "." for its internal directory -
  # if that's the case, we need to just set the various config output
  # directories to the same value.
  set(CFG_ROOT ${iMSTK_BINARY_DIR})

  foreach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    if(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
      set(CFG_ROOT ${iMSTK_BINARY_DIR}/${CFG_TYPE})
    endif()
    string(TOUPPER "${CFG_TYPE}" CFG_TYPE_UPPER)
    foreach(type LIBRARY RUNTIME ARCHIVE)
      if(DEFINED iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}
          AND NOT EXISTS ${iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
        message(FATAL_ERROR "iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} is set to a non-existing directory [${iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}]")
      endif()

      if(iMSTK_SUPERBUILD)
        set(output_dir ${iMSTK_BINARY_DIR}/${${type}_DIRNAME})
        if(NOT DEFINED iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER})
          set(iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} ${iMSTK_BINARY_DIR}/iMSTK-build/${${type}_DIRNAME})
        endif()
      else()
        if(NOT DEFINED iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER})
          set(output_dir ${iMSTK_BINARY_DIR}/${${type}_DIRNAME})
        else()
          set(output_dir ${iMSTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
        endif()
      endif()
      set(CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} ${output_dir} CACHE INTERNAL "Single output directory for building ${CFG_TYPE} libraries.")
    endforeach()

    if(NOT iMSTK_BINARY_DIR_${CFG_TYPE_UPPER})
      set(iMSTK_BINARY_DIR_${CFG_TYPE_UPPER} ${CFG_ROOT})
    endif()

    set(CMAKE_BINARY_DIR_${CFG_TYPE_UPPER} iMSTK_BINARY_DIR_${CFG_TYPE_UPPER} CACHE INTERNAL "Toplevel binary dir for ${CFG_TYPE} building.")

  endforeach()
endif()
