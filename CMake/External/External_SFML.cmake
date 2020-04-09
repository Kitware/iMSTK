#-----------------------------------------------------------------------------
# Add SFML External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( SFML
  URL https://github.com/SFML/SFML/archive/2.4.2.zip
  URL_MD5 b2e2cf6c43e9d2b578c4abd9aa28a6aa
  CMAKE_CACHE_ARGS
    -DSFML_BUILD_AUDIO:BOOL=ON
    -DSFML_BUILD_GRAPHICS:BOOL=OFF
    -DSFML_BUILD_NETWORK:BOOL=OFF
    -DSFML_BUILD_WINDOW:BOOL=OFF
    -DSFML_BUILD_EXAMPLES:BOOL=OFF
    -DSFML_GENERATE_PDB:BOOL=OFF
  #DEPENDENCIES ""
  #RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)
