#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( gli
  URL https://gitlab.kitware.com/iMSTK/gli/-/archive/0.8.2/gli-0.8.2.zip
  URL_MD5 6f891066d2365bbd58f27a71763ca335
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
