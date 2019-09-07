#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_define_external_dirs( SCCD )

if(MSVC)
  set(config_dir "$(Configuration)")
endif()

set(copy_sccd_headers_command
  ${CMAKE_COMMAND} -E copy_directory
  ${SCCD_SOURCE_DIR}/inc
  ${CMAKE_INSTALL_PREFIX}/include
  )
set(copy_sccd_lib_command
  ${CMAKE_COMMAND} -E copy
  ${SCCD_BINARY_DIR}/src/${config_dir}/sccd$<$<CONFIG:Debug>:d>.lib
  ${CMAKE_INSTALL_PREFIX}/lib
  )


imstk_add_external_project( SCCD
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/SCCD.git
  GIT_TAG 760854d53f4db6fd1d1a073aabd2d3ecb32506b5
  CMAKE_CACHE_ARGS
    -DBUILD_SAMPLE_APP:BOOL=OFF
  RELATIVE_INCLUDE_PATH "inc"
  INSTALL_COMMAND
    COMMAND ${copy_sccd_headers_command}
    COMMAND ${copy_sccd_lib_command}
  DEPENDENCIES
    Eigen
  #VERBOSE
  )
