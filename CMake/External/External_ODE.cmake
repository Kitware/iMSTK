#----------------------------------------------------------
# ODE does not have a cmake version.
# Use of this file is a workaround to create a mode for setting
# up iMSTK to build with ODE.
# This piece of code also sets other necessary directories.
# It is assumed that ODE was compiled with double precision.
#
# When ODE is configured with cmake (in future), 
# this script should be updated.
#----------------------------------------------------------

set(USE_SYSTEM_ODE 1 CACHE BOOL "Use system ODE" FORCE)

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( ODE
  DOWNLOAD_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
mark_as_superbuild(ODE_INCLUDE_DIR)
mark_as_superbuild(ODE_LIBRARY)

#-----------------------------------------------------------------------------
if(WIN32)
  # Since on windows there is no support for Runtime Path, we need to copy
  # the ODE shared library.
  if(NOT DEFINED ODE_SHARED_LIBRARY)
    get_filename_component(_dir ${ODE_LIBRARY} DIRECTORY)
    get_filename_component(_name_we ${ODE_LIBRARY} NAME_WE)
    set(ODE_SHARED_LIBRARY "${_dir}/${_name_we}.dll")
    if(NOT EXISTS "${ODE_SHARED_LIBRARY}")
      message(FATAL_ERROR "ODE_SHARED_LIBRARY corresponds to a nonexistent file. ODE_SHARED_LIBRARY is '${ODE_SHARED_LIBRARY}'")
    endif()
  endif()
  set(_dest "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/")
  ExternalProject_Add_Step( ODE copy_shared_library
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_dest}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ODE_SHARED_LIBRARY} ${_dest}
    COMMENT "Copy ODE shared library into ${_dest}"
    DEPENDERS  download
    USES_TERMINAL 1
    )
endif()
