
# Set TBB_SOURCE_DIR and TBB_PREFIX
imstk_define_external_dirs( TBB )

# Version used to copy files & directories, and download archive from GitHub
set(TBB_VER "2019_20191006oss")

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(TBB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

  set(copy_tbb_config_command
    ${CMAKE_COMMAND} -E copy
      ${TBB_SOURCE_DIR}/tbb${TBB_VER}/cmake/TBBConfig.cmake
      ${TBB_INSTALL_DIR}/cmake/TBBConfig.cmake
    )

  set(copy_tbb_headers_command
    ${CMAKE_COMMAND} -E copy_directory
      ${TBB_SOURCE_DIR}/tbb${TBB_VER}/include/tbb
      ${TBB_INSTALL_DIR}/include/tbb
    )

  if(WIN32)
    set(_subdir "intel64/vc14/")
  elseif(APPLE)
    set(_subdir "")
  elseif(UNIX)
    set(_subdir "intel64/gcc4.8/")
  endif()

  set(copy_tbb_libraries_command
    ${CMAKE_COMMAND} -E copy_directory
      ${TBB_SOURCE_DIR}/tbb${TBB_VER}/lib/${_subdir}
      ${TBB_INSTALL_DIR}/lib/${_subdir}
    )

  set(TBB_INSTALL_COMMAND
    INSTALL_COMMAND
      COMMAND ${copy_tbb_config_command}
      COMMAND ${copy_tbb_headers_command}
      COMMAND ${copy_tbb_libraries_command}
    )

  if(WIN32)
    set(copy_tbb_dll_command
      ${CMAKE_COMMAND} -E copy_directory
        ${TBB_SOURCE_DIR}/tbb${TBB_VER}/bin/${_subdir}
        ${TBB_INSTALL_DIR}/bin/${_subdir}
      )
    list(APPEND TBB_INSTALL_COMMAND
      COMMAND ${copy_tbb_dll_command}
      )
  endif()

else()
  set(TBB_INSTALL_COMMAND
    INSTALL_COMMAND
      COMMAND ${SKIP_STEP_COMMAND}
    )
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

if(WIN32)
  set(tbb_file "tbb${TBB_VER}_win.zip")
  set(tbb_md5 "a061a7c9821a374023201e8592860730")
elseif(APPLE)
  set(tbb_file "tbb${TBB_VER}_mac.tgz")
  set(tbb_md5 "43a0d6409317ee94f047622fd489a6c8")
else()
  set(tbb_file "tbb${TBB_VER}_lin.tgz")
  set(tbb_md5 "b5025847fa47040b4d2da8d6bdba0224")
endif()

imstk_add_external_project(TBB
  URL https://github.com/oneapi-src/oneTBB/releases/download/2019_U9/${tbb_file}
  URL_MD5 ${tbb_md5}
  DOWNLOAD_DIR ${TBB_PREFIX}
  SOURCE_DIR ${TBB_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  ${TBB_INSTALL_COMMAND}
  #RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)
