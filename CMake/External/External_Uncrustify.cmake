#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

include(imstkAddExternalProject)
imstk_add_external_project( Uncrustify
  GIT_REPOSITORY https://github.com/uncrustify/uncrustify.git
  GIT_TAG uncrustify-0.64
  CMAKE_CACHE_ARGS
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
