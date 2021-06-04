#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( GTest
   URL https://github.com/google/googletest/archive/53495a2a7d6ba7e0691a7f3602e9a5324bba6e45.zip
   URL_MD5 8349ef674d27b005a43ce3679cb04947
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
