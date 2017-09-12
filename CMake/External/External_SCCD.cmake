#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( SCCD
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/SCCD.git
  GIT_TAG 64550894016d84e80939a1c51ef6df5307c54fae
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_ARGS
    -DBUILD_SAMPLE_APP:BOOL=OFF
  RELATIVE_INCLUDE_PATH "inc"
  DEPENDENCIES
    Eigen
  #VERBOSE
  )
