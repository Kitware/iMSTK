
include(imstkAddExternalProject)

if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(Eigen3_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
# After the external project updates its CMake minimum required version
# to >= 3.15, package export is disabled by default (see policy CMP0090)
# and explicitly setting this options will not be needed.
list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
  -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY:BOOL=ON
  )

# Download options
if(NOT DEFINED iMSTK_Eigen3_GIT_SHA)
  set(iMSTK_Eigen3_GIT_SHA "3.4.0")
endif()
if(NOT DEFINED iMSTK_Eigen3_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL "https://gitlab.kitware.com/iMSTK/eigen/-/archive/${iMSTK_Eigen3_GIT_SHA}/eigen-${iMSTK_Eigen3_GIT_SHA}.tar.gz"
    URL_HASH MD5=4c527a9171d71a72a9d4186e65bea559
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_Eigen3_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_Eigen3_GIT_SHA}
    )
endif()

imstk_add_external_project( Eigen3
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
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
