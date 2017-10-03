#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v4.0.1
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_ARGS
    -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
    -DASSIMP_BUILD_TESTS:BOOL=OFF
    -DASSIMP_NO_EXPORT:BOOL=ON
    -DLIBRARY_SUFFIX:STRING=
  RELATIVE_INCLUDE_PATH "include"
  #DEPENDENCIES ""
  #VERBOSE
  )
