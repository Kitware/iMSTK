
include(imstkAddExternalProject)

if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(VegaFEM_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(VegaFEM_DEPENDENCIES "")
if(WIN32)
  list(APPEND VegaFEM_DEPENDENCIES PThreads)
endif(WIN32)

imstk_add_external_project( VegaFEM
  URL https://gitlab.kitware.com/iMSTK/vegafemv4.0/-/archive/build_model_reduction/vegafemv4.0-build_model_reduction.zip
  URL_MD5 3f04bb7c2ba080785bcadf44d1a462a3
  CMAKE_CACHE_ARGS
    -DVegaFEM_ENABLE_PTHREADS_SUPPORT:BOOL=ON
    -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=OFF
    -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=${${PROJECT_NAME}_USE_MODEL_REDUCTION}
    -DVegaFEM_BUILD_UTILITIES:BOOL=OFF
  DEPENDENCIES ${VegaFEM_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)

if(NOT USE_SYSTEM_VegaFEM)
  set(VegaFEM_DIR ${VegaFEM_INSTALL_DIR}/lib/cmake/VegaFEM)
  #message(STATUS "VegaFEM_DIR : ${VegaFEM_DIR}")
endif()

