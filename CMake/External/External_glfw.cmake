#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.2.1
  CMAKE_ARGS
    -DGLFW_BUILD_DOCS:BOOL=OFF
    -DGLFW_BUILD_EXAMPLES:BOOL=OFF
    -DGLFW_BUILD_TESTS:BOOL=OFF
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
