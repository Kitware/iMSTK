#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( SCCD
  GIT_REPOSITORY git@gitlab.kitware.com:iMSTK/SCCD.git
  GIT_TAG 7145cc437ff9f9744a8363736590260e2e660bb6
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_ARGS
    -DBUILD_SAMPLE_APP:BOOL=OFF
  RELATIVE_INCLUDE_PATH "inc"
  DEPENDENCIES
    Eigen
  #VERBOSE
  )
