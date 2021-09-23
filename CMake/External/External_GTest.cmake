#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( GTest
   URL https://github.com/google/googletest/archive/40dfd4b775a66979ad1bd19321cdfd0feadfea27.zip
   URL_MD5 222793254ade8010fe6ff000f18c6b15
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
