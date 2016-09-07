#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( PThreads
  GIT_REPOSITORY ${git_protocol}://github.com/songdongsheng/libpthread.git
  GIT_TAG 38821d529407dcb0b4661c6805d7b1bf83f3204d
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  RELATIVE_INCLUDE_PATH "include"
  DEPENDENCIES ""
  #VERBOSE
  )
