
include(imstkAddExternalProject)

if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(VegaFEM_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(VegaFEM_DEPENDENCIES "")

set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
# After the external project updates its CMake minimum required version
# to >= 3.15, package export is disabled by default (see policy CMP0090)
# and explicitly setting this options will not be needed.
list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
  -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY:BOOL=ON
  )

# Download options
if(NOT DEFINED iMSTK_VegaFEM_GIT_SHA)
  set(iMSTK_VegaFEM_GIT_SHA "b10672cf1493349087a1982c8b5bf775a5f4e415") # master
endif()
if(NOT DEFINED iMSTK_VegaFEM_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://gitlab.kitware.com/iMSTK/VegaFEM-CMake/-/archive/${iMSTK_VegaFEM_GIT_SHA}/VegaFEM-CMake-${iMSTK_VegaFEM_GIT_SHA}.zip
    URL_HASH MD5=d945353d89dab31b2691851a17926b60
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_VegaFEM_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_VegaFEM_GIT_SHA}
    )
endif()

imstk_add_external_project( VegaFEM
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
    -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=OFF
    -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=${${PROJECT_NAME}_USE_MODEL_REDUCTION}
    -DVegaFEM_BUILD_UTILITIES:BOOL=OFF
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
  DEPENDENCIES ${VegaFEM_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)

if(NOT USE_SYSTEM_VegaFEM)
  set(VegaFEM_DIR ${VegaFEM_INSTALL_DIR}/lib/cmake/VegaFEM)
  #message(STATUS "VegaFEM_DIR : ${VegaFEM_DIR}")
endif()

