#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(VegaFEM_DEPENDENCIES "")
if(WIN32)
  list(APPEND VegaFEM_DEPENDENCIES PThreads)
endif(WIN32)

include(imstkAddExternalProject)
imstk_add_external_project( VegaFEM
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/vegafemv4.0
  GIT_TAG f9c96c4128437a559e5fb9a93830ef3c9a627d5e
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
  #message(STATUS "VegaFEM_DIR : ${VegaFEM_DIR}")
endif()

