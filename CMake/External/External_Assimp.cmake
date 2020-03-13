#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Assimp
  URL https://gitlab.kitware.com/iMSTK/assimp/-/archive/fixCompilationError/assimp-fixCompilationError.zip
  URL_MD5 fab081af9dbd73a9a5c61cd33dd93340
  CMAKE_CACHE_ARGS
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
    -DASSIMP_BUILD_TESTS:BOOL=OFF
    -DASSIMP_NO_EXPORT:BOOL=ON
    -DLIBRARY_SUFFIX:STRING=
  RELATIVE_INCLUDE_PATH "include"
  #DEPENDENCIES ""
  #VERBOSE
  )