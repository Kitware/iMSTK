
include(imstkAddExternalProject)

if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(Eigen3_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
# After updating the CMake minimum required version to >= 3.15, package
# export is disabled by default (see policy CMP0090) and explicitly setting
# these options will not be needed.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DCMAKE_EXPORT_PACKAGE_REGISTRY:BOOL=OFF
    )
else()
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY:BOOL=ON
    )
endif()

set(git_sha "3.4.0")
imstk_add_external_project( Eigen3
  URL "https://gitlab.kitware.com/iMSTK/eigen/-/archive/${git_sha}/eigen-${git_sha}.tar.gz"
  URL_HASH MD5=4c527a9171d71a72a9d4186e65bea559
  CMAKE_CACHE_ARGS
        -DBUILD_TESTING:BOOL=OFF
        ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
)

if(NOT USE_SYSTEM_Eigen3)
  set(Eigen3_DIR ${Eigen3_INSTALL_DIR}/share/eigen3/cmake)
  #message(STATUS "Eigen3_DIR : ${Eigen3_DIR}")
endif()
