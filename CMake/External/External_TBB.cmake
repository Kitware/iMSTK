#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

# Download options
if(NOT DEFINED iMSTK_TBB_GIT_SHA)
  # Version 2021.5.0
  set(iMSTK_TBB_GIT_SHA "3df08fe234f23e732a122809b40eb129ae22733f")
endif()
if(NOT DEFINED iMSTK_TBB_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    # 2021.5
    URL https://github.com/oneapi-src/oneTBB/archive/${iMSTK_TBB_GIT_SHA}.zip
    URL_HASH MD5=8e42b09da31adb346d741f187f96594e
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_TBB_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_TBB_GIT_SHA}
    )
endif()

imstk_add_external_project( TBB
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
    -DTBB_TEST:BOOL=OFF
)
