#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

# Download options
if(NOT DEFINED iMSTK_TBB_GIT_SHA)
  # Version 2021.1.1
  set(iMSTK_TBB_GIT_SHA "46fb877ef1618d9de9a9ba10cee107592b7cdb2d")
endif()
if(NOT DEFINED iMSTK_TBB_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    # 2021.1
    URL https://github.com/oneapi-src/oneTBB/archive/${iMSTK_TBB_GIT_SHA}.zip
    URL_HASH MD5=1a0d5676ab568c8b9de57c7ecc082505
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
