#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

# Download options
if(NOT DEFINED iMSTK_Assimp_GIT_SHA)
  set(iMSTK_Assimp_GIT_SHA "fixCompilationError")
endif()
if(NOT DEFINED iMSTK_Assimp_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://gitlab.kitware.com/iMSTK/assimp/-/archive/${iMSTK_Assimp_GIT_SHA}/assimp-${iMSTK_Assimp_GIT_SHA}.zip
    URL_HASH MD5=fab081af9dbd73a9a5c61cd33dd93340
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_Assimp_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_Assimp_GIT_SHA}
    )
endif()

imstk_add_external_project( Assimp
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
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
