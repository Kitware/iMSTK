#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
set(LIBNIFALCON_DEPENDENCIES "")
if(WIN32)
  list(APPEND LIBNIFALCON_DEPENDENCIES "FTD2XX")
endif(WIN32)

include(imstkAddExternalProject)
imstk_add_external_project( LibNiFalcon
  GIT_REPOSITORY https://github.com/agirault/libnifalcon
  GIT_TAG 47586d4e21c987fb9e3ce822bf707c53e8fc4e52
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_ARGS
    -DBUILD_TESTING:BOOL=OFF
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SWIG_BINDINGS:BOOL=OFF
    -DBUILD_SHARED:BOOL=OFF
  DEPENDENCIES ${LIBNIFALCON_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH "include"
  #VERBOSE
  )
