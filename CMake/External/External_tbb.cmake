#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project(tbb
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/TBB-CMake.git
  GIT_TAG b066defc0229a1e92d7a200eb3fe0f7e35945d95
  CMAKE_CACHE_ARGS
    -DTBB_BUILD_TESTS:BOOL=OFF
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )