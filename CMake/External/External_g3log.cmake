#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

include(imstkAddExternalProject)

# Set g3log_SOURCE_DIR and g3log_BINARY_DIR
imstk_define_external_dirs( g3log )

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(g3log_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

if(MSVC)
  set(g3log_built_library ${CMAKE_COMMAND} -E copy
                          ${g3log_BINARY_DIR}/$(Configuration)/g3logger$<$<CONFIG:Debug>:d>.lib
                          ${g3log_INSTALL_DIR}/lib/g3logger$<$<CONFIG:Debug>:d>.lib)
  set(g3log_built_shared)
else()
  set(g3log_built_library ${CMAKE_COMMAND} -E copy
                          ${g3log_BINARY_DIR}/libg3logger.a
                          ${g3log_INSTALL_DIR}/lib/libg3logger.a)
  set(g3log_built_shared ${CMAKE_COMMAND} -E copy
                         ${g3log_BINARY_DIR}/libg3logger.so
                         ${g3log_INSTALL_DIR}/lib/libg3logger.so)
endif()

set(g3log_INSTALL_COMMAND
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${g3log_SOURCE_DIR}/src/g3log ${g3log_INSTALL_DIR}/include/g3log
    COMMAND ${g3log_built_library}
    COMMAND ${g3log_built_shared}
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
imstk_add_external_project( g3log
  URL https://gitlab.kitware.com/iMSTK/g3log/-/archive/6c1698c4f7db6b9e4246ead38051f9866ea3ac06/archive.zip
  URL_MD5 3815bbfec2ff51dc473063c35eec0f36
  CMAKE_CACHE_ARGS
    -DADD_FATAL_EXAMPLE:BOOL=OFF
  ${g3log_INSTALL_COMMAND}
  RELATIVE_INCLUDE_PATH "src"
  DEPENDENCIES ""
  #VERBOSE
)
