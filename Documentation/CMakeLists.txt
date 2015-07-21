####### SimMedTK Documentation
## Reference Documentation
#
# If we have doxygen, create reference API documentation for SimMedTK.
#
# This can also generate tag-files and documentation for third-party
# libraries for cross-referencing with SimMedTK's docs.
#
if(DOXYGEN_FOUND)
  file(MAKE_DIRECTORY "${SimMedTK_BINARY_DIR}/documentation/reference")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SimMedTK.doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/SimMedTK.doxyfile
    @ONLY
  )
  add_custom_command(
    OUTPUT ${SimMedTK_BINARY_DIR}/doc/reference/SimMedTK.tags
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/SimMedTK.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/SimMedTK.doxyfile"
      # TODO: Add tag files for external dependencies here.
    COMMENT "Generating SimMedTK API documentation with Doxygen" VERBATIM
  )
  add_custom_target(doc
    DEPENDS ${SimMedTK_BINARY_DIR}/doc/reference/SimMedTK.tags
  )
endif(DOXYGEN_FOUND)

## End-user Documentation
#
# If we have rst2html, create the user's guide for SimMedTK
# as an HTML document.

# Define a macro for processing reStructuredText files
# if docutils were found.
if (SPHINX_FOUND)
  include(SimMedTKSphinxDocs)
  set(SimMedTK_USERGUIDE_DOCS
    index.rst
    userguide/index.rst
    userguide/contributing.rst
    userguide/administration.rst
  )
  if (DOXYGEN_FOUND)
    # Need doxygen docs built if possible
    list(APPEND SimMedTK_USERGUIDE_DOCS
      ${SimMedTK_BINARY_DIR}/documentation/reference/SimMedTK.tags)
  endif()

  set(SimMedTK_USERGUIDE_FIGS
    #userguide/figures/architecture.svg
  )
  # Add the top-level reStructuredText file.
  # All others are included by it.
  simmedtk_add_doc(userguide
    DEPENDS ${SimMedTK_USERGUIDE_DOCS}
    FIGURES ${SimMedTK_USERGUIDE_FIGS}
    BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/user
    DESTINATION share/doc/SimMedTK/userguide
  )
endif()
