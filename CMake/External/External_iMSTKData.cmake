#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(iMSTKData_PREFIX ${CMAKE_BINARY_DIR}/External/iMSTKData)

find_package(GitLFS QUIET)
if (NOT GitLFS_FOUND)
  message(FATAL_ERROR " Building the tests or the examples requires the iMSTK data, which in turn\
  requires git-lfs, make sure it is installed on your system, on linux please run `sudo apt-get install git-lfs`")
endif()

set(copy_data_command
  ${CMAKE_COMMAND} -E copy_directory
  ${iMSTKData_PREFIX}/src/Data
  ${CMAKE_INSTALL_PREFIX}/data
  )

include(imstkAddExternalProject)
imstk_add_external_project( iMSTKData
  GIT_REPOSITORY  "https://gitlab.kitware.com/iMSTK/imstk-data.git"
  GIT_TAG  "14824e3d53328ed6be481981959780f32881030b"
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND COMMAND ${copy_data_command}
  DEPENDENCIES ""
  #VERBOSE
)

