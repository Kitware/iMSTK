#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(iMSTKData_PREFIX ${CMAKE_BINARY_DIR}/External/iMSTKData)
set(iMSTKData_SOURCE ${iMSTKData_PREFIX}/src/Data CACHE INTERNAL "")

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

# HS - Due to an issue where it seems that the repository does not get updated
# we need to call fetch here to refresh the index, this way the checkout can 
# succeed for the add_external_project call
execute_process(
  COMMAND ${GIT_EXECUTABLE} fetch --all
  WORKING_DIRECTORY ${iMSTKData_PREFIX}/src
  RESULT_VARIABLE error_code
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

