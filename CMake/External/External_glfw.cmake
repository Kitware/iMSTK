#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( glfw
  URL https://github.com/glfw/glfw/archive/3.2.1.zip
  URL_MD5 4d5b70be0926d17de2ce772762dcfa411b04709d
  CMAKE_ARGS
    -DGLFW_BUILD_DOCS:BOOL=OFF
    -DGLFW_BUILD_EXAMPLES:BOOL=OFF
    -DGLFW_BUILD_TESTS:BOOL=OFF
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
