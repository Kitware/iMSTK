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
  ${CMAKE_INSTALL_PREFIX}/include/sccd
  )
if(WIN32)
  set(copy_sccd_lib_command
    ${CMAKE_COMMAND} -E copy
    ${SCCD_BINARY_DIR}/src/${config_dir}/sccd$<$<CONFIG:Debug>:d>.lib
    ${CMAKE_INSTALL_PREFIX}/lib
    )
else()
  set(copy_sccd_lib_command
    ${CMAKE_COMMAND} -E copy
    ${SCCD_BINARY_DIR}/src/${config_dir}/libsccd$<$<CONFIG:Debug>:d>.a
    ${CMAKE_INSTALL_PREFIX}/lib
    )
endif()


imstk_add_external_project( SCCD
  URL https://gitlab.kitware.com/iMSTK/SCCD/-/archive/addressWarnings/SCCD-addressWarnings.zip
  URL_MD5 a45b33df99ea3e7ad85340afe8dcc489
  CMAKE_CACHE_ARGS
    -DBUILD_SAMPLE_APP:BOOL=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
  RELATIVE_INCLUDE_PATH "inc"
  INSTALL_COMMAND
    COMMAND ${copy_sccd_headers_command}
    COMMAND ${copy_sccd_lib_command}
  DEPENDENCIES
    Eigen3
  #VERBOSE
)
