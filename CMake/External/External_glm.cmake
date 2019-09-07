#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( glm
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/glm.git
  GIT_TAG 0.9.8.3
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
