#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( GTest
#  URL https://gitlab.kitware.com/iMSTK/googletest/-/archive/release-1.10.0/googletest-release-1.10.0.zip
#  URL_MD5 1878d0300f865cb54df06b220ded5168
   URL https://github.com/google/googletest/archive/refs/heads/master.zip
#  URL_MD5 1878d0300f865cb54df06b220ded5168
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
