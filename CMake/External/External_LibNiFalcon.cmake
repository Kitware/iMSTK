#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(LIBNIFALCON_DEPENDENCIES "")
if(WIN32)
  list(APPEND LIBNIFALCON_DEPENDENCIES "FTD2XX")
endif(WIN32)

# Download options
if(NOT DEFINED iMSTK_LibNiFalcon_GIT_SHA)
  set(iMSTK_LibNiFalcon_GIT_SHA "libusb1-windows")
endif()
if(NOT DEFINED iMSTK_LibNiFalcon_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://gitlab.kitware.com/iMSTK/libnifalcon/-/archive/${iMSTK_LibNiFalcon_GIT_SHA}/libnifalcon-${iMSTK_LibNiFalcon_GIT_SHA}.zip
    URL_HASH MD5=6d5d68c92837388bfcd27f99a48b921d
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_LibNiFalcon_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_LibNiFalcon_GIT_SHA}
    )
endif()

include(imstkAddExternalProject)
imstk_add_external_project( LibNiFalcon
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SWIG_BINDINGS:BOOL=OFF
    -DBUILD_SHARED:BOOL=OFF
  DEPENDENCIES ${LIBNIFALCON_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH "include/ftd2xx"
  #VERBOSE
)
