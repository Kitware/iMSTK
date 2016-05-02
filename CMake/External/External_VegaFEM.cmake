#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(VegaFEM_DEPENDENCIES "")
if(WIN32)
  list(APPEND VegaFEM_DEPENDENCIES PThreads)
endif(WIN32)

include(imstkAddExternalProject)
imstk_add_external_project( VegaFEM
  REPOSITORY git@gitlab.kitware.com:iMSTK/VegaFEM-CMake.git
  GIT_TAG iMSTK
  CMAKE_ARGS
    -DVegaFEM_ENABLE_PTHREADS_SUPPORT:BOOL=ON
    -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=OFF
    -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=OFF
    -DVegaFEM_BUILD_UTILITIES:BOOL=ON
  DEPENDENCIES ${VegaFEM_DEPENDENCIES}
  #RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
