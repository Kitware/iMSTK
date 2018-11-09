#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( PThreads
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/libpthread.git
  GIT_TAG 40707ceea7c35b06f7448f61ae62422efe6a7873
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
  )