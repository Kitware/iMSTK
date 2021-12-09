#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

# Download options
if(NOT DEFINED iMSTK_GTest_GIT_SHA)
  set(iMSTK_GTest_GIT_SHA "40dfd4b775a66979ad1bd19321cdfd0feadfea27")
endif()
if(NOT DEFINED iMSTK_GTest_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://github.com/google/googletest/archive/${iMSTK_GTest_GIT_SHA}.zip
    URL_HASH MD5=222793254ade8010fe6ff000f18c6b15
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_GTest_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_GTest_GIT_SHA}
    )
endif()

imstk_add_external_project( GTest
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
    -DBUILD_GMOCK:BOOL=ON
    -DBUILD_GTEST:BOOL=ON
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -Dgtest_force_shared_crt:BOOL=ON
  DEPENDENCIES ""
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )

if(NOT USE_SYSTEM_GTest)
  set(GTest_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/GTest)
  #message(STATUS "GTest_DIR : ${GTest_DIR}")
endif()
