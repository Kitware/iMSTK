#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( glm
  URL https://gitlab.kitware.com/iMSTK/glm/-/archive/0.9.8.3/glm-0.9.8.3.zip
  URL_MD5 a95fae241b7ff3ba7d6e614cfd87df02
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
)
