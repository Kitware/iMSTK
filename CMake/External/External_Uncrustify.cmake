#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

# Download options
if(NOT DEFINED iMSTK_Uncrustify_GIT_SHA)
  set(iMSTK_Uncrustify_GIT_SHA "0.70.1")
endif()
if(NOT DEFINED iMSTK_Uncrustify_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://github.com/uncrustify/uncrustify/archive/uncrustify-${iMSTK_Uncrustify_GIT_SHA}.zip
    URL_HASH MD5=0f6bd2ca1a35efebac3bc8aee44f6532
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_Uncrustify_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_Uncrustify_GIT_SHA}
    )
endif()

include(imstkAddExternalProject)
imstk_add_external_project( Uncrustify
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
