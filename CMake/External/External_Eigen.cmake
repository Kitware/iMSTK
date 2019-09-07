#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Eigen
  GIT_REPOSITORY https://github.com/eigenteam/eigen-git-mirror.git
  GIT_TAG 3.3.3
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
