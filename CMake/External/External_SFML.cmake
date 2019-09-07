#-----------------------------------------------------------------------------
# Add SFML External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.4.2
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