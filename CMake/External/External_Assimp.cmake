#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

# Download options
if(NOT DEFINED iMSTK_Assimp_GIT_SHA)
  set(iMSTK_Assimp_GIT_SHA "9c4296940da7f767dc08ccc11de55ea02dfce991") # imstk-v3.3.1-2016-07-08-a8673d482
endif()
if(NOT DEFINED iMSTK_Assimp_GIT_REPOSITORY)
  set(iMSTK_Assimp_GIT_REPOSITORY "https://gitlab.kitware.com/iMSTK/assimp.git")
endif()

imstk_add_external_project( Assimp
  GIT_REPOSITORY ${iMSTK_Assimp_GIT_REPOSITORY}
  GIT_TAG ${iMSTK_Assimp_GIT_SHA}
  CMAKE_CACHE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
    -DASSIMP_BUILD_TESTS:BOOL=OFF
    -DASSIMP_NO_EXPORT:BOOL=ON
    -DLIBRARY_SUFFIX:STRING=
  RELATIVE_INCLUDE_PATH "include"
  #DEPENDENCIES ""
  #VERBOSE
)
