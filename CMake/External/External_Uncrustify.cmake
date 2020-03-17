#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

include(imstkAddExternalProject)
imstk_add_external_project( Uncrustify
  URL https://github.com/uncrustify/uncrustify/archive/uncrustify-0.70.1.zip
  URL_MD5 0f6bd2ca1a35efebac3bc8aee44f6532
  CMAKE_CACHE_ARGS
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
