#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( PThreads
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/libpthread.git
  GIT_TAG 660654b332126834a20db43ca649e43fdcd32016
  # This is the iMSTK branch
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
  )