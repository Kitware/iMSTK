#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(VegaFEM_DEPENDENCIES "")
if(WIN32)
  list(APPEND VegaFEM_DEPENDENCIES PThreads)
endif(WIN32)

include(imstkAddExternalProject)
imstk_add_external_project( VegaFEM
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/VegaFEM-CMake.git
  GIT_TAG 4d5b70be0926d17de2ce772762dcfa411b04709d
  CMAKE_CACHE_ARGS
    -DVegaFEM_ENABLE_PTHREADS_SUPPORT:BOOL=ON
    -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=OFF
    -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=OFF
    -DVegaFEM_BUILD_UTILITIES:BOOL=OFF
  DEPENDENCIES ${VegaFEM_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
