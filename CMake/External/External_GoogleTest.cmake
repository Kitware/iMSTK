#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( GoogleTest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG dc043e1ca6bd509a92452ed54e817b6979869372
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_CACHE_ARGS
    -DBUILD_GMOCK:BOOL=ON
    -DBUILD_GTEST:BOOL=ON
    -DBUILD_SHARED_LIBS:BOOL=ON
  DEPENDENCIES ""
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
