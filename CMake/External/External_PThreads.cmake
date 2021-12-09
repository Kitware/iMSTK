#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
set(git_sha "iMSTK")
imstk_add_external_project( PThreads
  URL https://gitlab.kitware.com/iMSTK/libpthread/-/archive/${git_sha}/libpthread-${git_sha}.zip
  URL_MD5 9bc417d5ebb8ac0859eeb77a147569ba
  # This is the iMSTK branch
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
)
