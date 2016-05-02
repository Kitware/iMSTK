#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( g3log
  REPOSITORY ${git_protocol}://github.com/KjellKod/g3log.git
  GIT_TAG 6c1698c4f7db6b9e4246ead38051f9866ea3ac06
  RELATIVE_INCLUDE_PATH "/src"
  #DEPENDENCIES ""
  #VERBOSE
  )
