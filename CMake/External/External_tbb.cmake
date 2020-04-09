#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project(tbb
  URL https://gitlab.kitware.com/iMSTK/TBB-CMake/-/archive/master/TBB-CMake-master.zip
  URL_MD5 cf8edc032dc473668c6b3109cfee19e8
  CMAKE_CACHE_ARGS
    -DTBB_BUILD_TESTS:BOOL=OFF
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)
