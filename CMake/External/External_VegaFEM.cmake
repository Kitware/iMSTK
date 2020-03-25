#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(VegaFEM_DEPENDENCIES "")
if(WIN32)
  list(APPEND VegaFEM_DEPENDENCIES PThreads)
endif(WIN32)

include(imstkAddExternalProject)
imstk_add_external_project( VegaFEM
  URL https://gitlab.kitware.com/iMSTK/VegaFEM-CMake/-/archive/iMSTK/VegaFEM-CMake-iMSTK.zip
  URL_MD5 11d1f47599b6d70f9b6dc6391bed7a20
  CMAKE_CACHE_ARGS
    -DVegaFEM_ENABLE_PTHREADS_SUPPORT:BOOL=ON
    -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=OFF
    -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=OFF
    -DVegaFEM_BUILD_UTILITIES:BOOL=OFF
  DEPENDENCIES ${VegaFEM_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)
if(NOT USE_SYSTEM_VegaFEM)
  set(VegaFEM_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/VegaFEM)
  message(STATUS "VegaFEM_DIR : ${VegaFEM_DIR}")
endif()

