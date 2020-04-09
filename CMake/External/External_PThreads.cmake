#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( PThreads
  URL https://gitlab.kitware.com/iMSTK/libpthread/-/archive/iMSTK/libpthread-iMSTK.zip
  URL_MD5 9bc417d5ebb8ac0859eeb77a147569ba
  # This is the iMSTK branch
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
)
