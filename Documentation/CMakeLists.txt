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

####### iMSTK Documentation
## Reference Documentation
#
# If we have doxygen, create reference API documentation for iMSTK.
#
# This can also generate tag-files and documentation for third-party
# libraries for cross-referencing with iMSTK's docs.
#
if(DOXYGEN_FOUND)
  file(MAKE_DIRECTORY "${iMSTK_BINARY_DIR}/documentation/reference")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/iMSTK.doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/iMSTK.doxyfile
    @ONLY
  )
  add_custom_command(
    OUTPUT ${iMSTK_BINARY_DIR}/doc/reference/iMSTK.tags
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/iMSTK.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/iMSTK.doxyfile"
      # TODO: Add tag files for external dependencies here.
    COMMENT "Generating iMSTK API documentation with Doxygen" VERBATIM
  )
  add_custom_target(doc
    DEPENDS ${iMSTK_BINARY_DIR}/doc/reference/iMSTK.tags
  )
endif(DOXYGEN_FOUND)

## End-user Documentation
#
# If we have rst2html, create the user's guide for iMSTK
# as an HTML document.

# Define a macro for processing reStructuredText files
# if docutils were found.
if (SPHINX_FOUND)
  include(iMSTKSphinxDocs)
  set(iMSTK_USERGUIDE_DOCS
    index.rst
    userguide/index.rst
    userguide/contributing.rst
    userguide/administration.rst
  )
  if (DOXYGEN_FOUND)
    # Need doxygen docs built if possible
    list(APPEND iMSTK_USERGUIDE_DOCS
      ${iMSTK_BINARY_DIR}/documentation/reference/iMSTK.tags)
  endif()

  set(iMSTK_USERGUIDE_FIGS
    #userguide/figures/architecture.svg
  )
  # Add the top-level reStructuredText file.
  # All others are included by it.
  imstk_add_doc(userguide
    DEPENDS ${iMSTK_USERGUIDE_DOCS}
    FIGURES ${iMSTK_USERGUIDE_FIGS}
    BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/user
    DESTINATION share/doc/iMSTK/userguide
  )
endif()
