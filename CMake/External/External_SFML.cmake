#-----------------------------------------------------------------------------
# Add SFML External Project
#-----------------------------------------------------------------------------

# Download options
if(NOT DEFINED iMSTK_SFML_GIT_SHA)
  set(iMSTK_SFML_GIT_SHA "2.4.2")
endif()
if(NOT DEFINED iMSTK_SFML_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://github.com/SFML/SFML/archive/${iMSTK_SFML_GIT_SHA}.zip
    URL_HASH MD5=b2e2cf6c43e9d2b578c4abd9aa28a6aa
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_SFML_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_SFML_GIT_SHA}
    )
endif()

include(imstkAddExternalProject)
imstk_add_external_project( SFML
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
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
