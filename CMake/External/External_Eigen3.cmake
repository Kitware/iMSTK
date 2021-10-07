#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Eigen3
  URL "https://gitlab.kitware.com/iMSTK/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz"
  URL_HASH MD5=4c527a9171d71a72a9d4186e65bea559
  CMAKE_CACHE_ARGS
        -DBUILD_TESTING:BOOL=OFF
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
)
if(NOT USE_SYSTEM_Eigen3)
  set(Eigen3_DIR ${CMAKE_INSTALL_PREFIX}/share/eigen3/cmake)
  #message(STATUS "Eigen3_DIR : ${Eigen3_DIR}")
endif()
