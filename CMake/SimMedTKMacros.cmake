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
  cmake_parse_arguments(target "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  set(libtype)
  if (WIN32)
    set(libtype STATIC)
  endif()
  add_library(${target} ${libtype} ${target_SOURCES})
  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/v${SimMedTK_VERSION}>
      $<INSTALL_INTERFACE:include/v${SimMedTK_VERSION}/${target}>
    )
  if (target_PUBLIC_HEADERS)
    simmedtk_install_library(${target})
    install(FILES ${target_PUBLIC_HEADERS}
      DESTINATION include/v${SimMedTK_VERSION}/${target}
      COMPONENT Development
    )
  endif()
endfunction()

macro(configure_msvc_runtime)
  # Default static linkage runtime
  set(MSVC_RUNTIME "static")
  if(BUILD_SHARED_LIBS)
    set(MSVC_RUNTIME "dll")
  endif()
  # Set compiler options.
  set(variables)
  foreach(type ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${type} type_UP)
    list(APPEND variables CMAKE_C_FLAGS_${type_UP})
    list(APPEND variables CMAKE_CXX_FLAGS_${type_UP})
  endforeach()
  if(${MSVC_RUNTIME} STREQUAL "static")
    message(STATUS
      "iMSTK - forcing use of statically-linked runtime."
    )
    foreach(variable ${variables})
      if(${variable} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        set(${variable} "${${variable}}" CACHE STRING "Compiler flags" FORCE)
      endif()
    endforeach()
  else()
    message(STATUS
      "iMSTK - forcing use of dynamically-linked runtime."
    )
    foreach(variable ${variables})
      if(${variable} MATCHES "/MT")
        string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        set(${variable} "${${variable}}" CACHE STRING "Compiler flags" FORCE)
      endif()
    endforeach()
  endif()
endmacro()

# Download url to local filename
function(DOWNLOAD_DATA url filename)
    # If the file already exist then skip this function
    if(EXISTS ${filename})
        return()
    endif()

    # Set data directorty to the file path
    get_filename_component(data_directory ${filename} DIRECTORY)

    # Create a Matrix directory to store the matrix data
    if(NOT EXISTS ${data_directory})
        file(MAKE_DIRECTORY ${data_directory})
    endif()

    # Download file
    file(DOWNLOAD "${url}" "${filename}"
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
        LOG log)
    list(GET DOWNLOAD_STATUS 0 STATUS0)
    if(NOT STATUS0 STREQUAL "0")
        list(GET DOWNLOAD_STATUS 1 STATUS1)
        message("${STATUS1}")
    endif()
endfunction(DOWNLOAD_DATA)


function(MIDAS_DOWNLOAD MIDAS_URL FILE_SHA1 LOCAL_FILE)
    if(NOT EXISTS ${LOCAL_FILE})
        file(
            DOWNLOAD "${MIDAS_URL}"
            "${LOCAL_FILE}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
            TIMEOUT 15
            TLS_VERIFY ON
            EXPECTED_HASH SHA1=${FILE_SHA1}
        )
        list(GET DOWNLOAD_STATUS 0 STATUS0)
        if(NOT STATUS0 STREQUAL "0")
            list(GET DOWNLOAD_STATUS 1 STATUS1)
            message("${STATUS1}")
        endif()
    endif()
endfunction()
