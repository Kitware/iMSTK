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

# - This module looks for Sphinx
# Find the Sphinx documentation generator
#
# This modules defines
# SPHINX_EXECUTABLE
# SPHINX_FOUND
find_program(SPHINX_EXECUTABLE
  NAMES sphinx-build
  PATHS
    /usr/bin
    /usr/local/bin
    /opt/local/bin
  DOC "Sphinx documentation generator"
)

if( NOT SPHINX_EXECUTABLE )
  set(_Python_VERSIONS
    2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5
  )
  foreach( _version ${_Python_VERSIONS} )
    set( _sphinx_NAMES sphinx-build-${_version} )
    find_program( SPHINX_EXECUTABLE
      NAMES ${_sphinx_NAMES}
      PATHS
        /usr/bin
        /usr/local/bin
        /opt/loca/bin
      DOC "Sphinx documentation generator"
    )
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
mark_as_advanced(SPHINX_EXECUTABLE)

function(Sphinx_add_target target_name builder conf source destination)

  add_custom_target(${target_name} ALL
    COMMAND ${SPHINX_EXECUTABLE} -b ${builder}
    -c ${conf}
    ${source}
    ${destination}
    COMMENT "Generating sphinx documentation: ${builder}"
  )

  set_property(DIRECTORY
    APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${destination}
  )

endfunction()
