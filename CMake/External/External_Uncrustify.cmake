#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Uncrustify
  GIT_REPOSITORY ${git_protocol}://github.com/uncrustify/uncrustify.git
  GIT_TAG uncrustify-0.62
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<BINARY_DIR> #TODO: does not work on Windows, see --host= option
  INSTALL_COMMAND cp <BINARY_DIR>/src/uncrustify ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )