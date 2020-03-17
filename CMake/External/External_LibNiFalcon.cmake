#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(LIBNIFALCON_DEPENDENCIES "")
if(WIN32)
  list(APPEND LIBNIFALCON_DEPENDENCIES "FTD2XX")
endif(WIN32)

include(imstkAddExternalProject)
imstk_add_external_project( LibNiFalcon
  URL https://gitlab.kitware.com/iMSTK/libnifalcon/-/archive/libusb1-windows/libnifalcon-libusb1-windows.zip
  URL_MD5 6d5d68c92837388bfcd27f99a48b921d
  CMAKE_CACHE_ARGS
    -DBUILD_TESTING:BOOL=OFF
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SWIG_BINDINGS:BOOL=OFF
    -DBUILD_SHARED:BOOL=OFF
  DEPENDENCIES ${LIBNIFALCON_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH "include/ftd2xx"
  #VERBOSE
  )