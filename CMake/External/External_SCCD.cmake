#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( SCCD
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/SCCD.git
  GIT_TAG 760854d53f4db6fd1d1a073aabd2d3ecb32506b5
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_CACHE_ARGS
    -DBUILD_SAMPLE_APP:BOOL=OFF
  RELATIVE_INCLUDE_PATH "inc"
  DEPENDENCIES
    Eigen
  #VERBOSE
  )
