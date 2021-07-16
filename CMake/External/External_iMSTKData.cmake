#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(iMSTKData_PREFIX ${CMAKE_BINARY_DIR}/External/iMSTKData)

set(copy_data_command
  ${CMAKE_COMMAND} -E copy_directory
  ${iMSTKData_PREFIX}/src/Data
  ${CMAKE_INSTALL_PREFIX}/data
  )

# HS - Due to an issue where it seems that the repository does not get updated
# we need to call fetch here to refresh the index, this way the checkout can 
# succeed for the add_external_project call
execute_process(
  COMMAND ${GIT_COMMAND} fetch --
  WORKING_DIRECTORY ${iMSTKData_PREFIX}/src
  RESULT_VARIABLE error_code
  )

include(imstkAddExternalProject)
imstk_add_external_project( iMSTKData
  GIT_REPOSITORY  "https://gitlab.kitware.com/iMSTK/imstk-data.git"
  GIT_TAG  "cd4cd59bcf86e4a9fed47e145e482990390e3c09"
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND COMMAND ${copy_data_command}
  DEPENDENCIES ""
  #VERBOSE
)

