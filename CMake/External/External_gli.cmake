#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( gli
  URL https://gitlab.kitware.com/iMSTK/gli/-/archive/0.8.2/gli-0.8.2.zip
  URL_MD5 4d5b70be0926d17de2ce772762dcfa411b04709d
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
