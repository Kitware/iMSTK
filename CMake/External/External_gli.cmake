#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( gli
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/gli.git
  GIT_TAG 0.8.2
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
