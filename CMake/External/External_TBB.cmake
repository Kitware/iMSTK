#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project(tbb
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/TBB-CMake.git
  GIT_TAG tbb44u4  
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_CACHE_ARGS
    -DTBB_BUILD_TESTS:BOOL=OFF
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )