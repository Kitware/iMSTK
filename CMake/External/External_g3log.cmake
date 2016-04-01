#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( g3log
  REPOSITORY ${git_protocol}://github.com/KjellKod/g3log.git
  GIT_TAG 579579962c94213a46fadbb70a996f08c19be3a2
  RELATIVE_INCLUDE_PATH "/src/g3log"
  #DEPENDENCIES ""
  #VERBOSE
  )
