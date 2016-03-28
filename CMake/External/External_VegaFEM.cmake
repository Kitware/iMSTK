#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( VegaFEM
  REPOSITORY git@gitlab.kitware.com:iMSTK/VegaFEM-CMake.git
  GIT_TAG simmedtk
  CMAKE_ARGS
    -DVegaFEM_ENABLE_PTHREADS_SUPPORT:BOOL=ON
    -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=OFF
    -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=OFF
    -DVegaFEM_BUILD_UTILITIES:BOOL=ON
  DEPENDENCIES ""
  #VERBOSE
  )
