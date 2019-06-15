#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project(tbb
  GIT_REPOSITORY https://github.com/wjakob/tbb.git
  GIT_TAG tbb44u4  
  CMAKE_CACHE_ARGS
    -DTBB_BUILD_TESTS:BOOL=OFF
  RELATIVE_INCLUDE_PATH "include/"
  #VERBOSE
  )