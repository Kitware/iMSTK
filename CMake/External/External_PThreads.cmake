#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( PThreads
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/libpthread.git
  GIT_TAG 26bdc33be9786a9ffbfcec806c857ea208c37201
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
  )