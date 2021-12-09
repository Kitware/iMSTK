#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

# Download options
if(NOT DEFINED iMSTK_PThreads_GIT_SHA)
  set(iMSTK_PThreads_GIT_SHA "iMSTK")
endif()
if(NOT DEFINED iMSTK_PThreads_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://gitlab.kitware.com/iMSTK/libpthread/-/archive/${iMSTK_PThreads_GIT_SHA}/libpthread-${iMSTK_PThreads_GIT_SHA}.zip
    URL_HASH MD5=9bc417d5ebb8ac0859eeb77a147569ba
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_PThreads_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_PThreads_GIT_SHA}
    )
endif()

imstk_add_external_project( PThreads
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
)
