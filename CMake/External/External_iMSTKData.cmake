#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(iMSTKData_PREFIX ${CMAKE_BINARY_DIR}/External/iMSTKData)

set(copy_data_command
  ${CMAKE_COMMAND} -E copy_directory
  ${iMSTKData_PREFIX}/src/Data
  ${CMAKE_INSTALL_PREFIX}/data
  )


include(imstkAddExternalProject)
imstk_add_external_project( iMSTKData
  GIT_REPOSITORY  "https://gitlab.kitware.com/iMSTK/imstk-data.git"
  GIT_TAG  "69d0d66ed040061abf176af41c3c3d1742203cbb"
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND COMMAND ${copy_data_command}
  DEPENDENCIES ""
  #VERBOSE
)

