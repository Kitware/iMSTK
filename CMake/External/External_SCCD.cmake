#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( SCCD
  GIT_REPOSITORY git@gitlab.kitware.com:iMSTK/SCCD.git
  GIT_TAG iMSTK
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_ARGS
    -DBUILD_SAMPLE_APP:BOOL=OFF
  RELATIVE_INCLUDE_PATH "inc"
  DEPENDENCIES
    Eigen
  #VERBOSE
  )
