#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

include(imstkAddExternalProject)
imstk_define_external_dirs( g3log )

if(MSVC)
  set(g3log_built_library ${CMAKE_COMMAND} -E copy 
                          ${g3log_BINARY_DIR}/$(Configuration)/g3logger$<$<CONFIG:Debug>:d>.lib
                          ${CMAKE_INSTALL_PREFIX}/lib/)
  set(g3log_built_shared)
else()
  set(g3log_built_library ${CMAKE_COMMAND} -E copy 
                          ${g3log_BINARY_DIR}/libg3logger.a
                          ${CMAKE_INSTALL_PREFIX}/lib/)
  set(g3log_built_shared ${CMAKE_COMMAND} -E copy 
                         ${g3log_BINARY_DIR}/libg3logger.so
                         ${CMAKE_INSTALL_PREFIX}/lib/)
endif()
set(g3log_hash 6c1698c4f7db6b9e4246ead38051f9866ea3ac06)

imstk_add_external_project( g3log
  URL https://gitlab.kitware.com/iMSTK/g3log/-/archive/6c1698c4f7db6b9e4246ead38051f9866ea3ac06/archive.zip
  URL_MD5 3815bbfec2ff51dc473063c35eec0f36
  CMAKE_CACHE_ARGS
    -DADD_FATAL_EXAMPLE:BOOL=OFF
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${g3log_SOURCE_DIR}/src/g3log ${CMAKE_INSTALL_PREFIX}/include/g3log
    COMMAND ${g3log_built_library}
    COMMAND ${g3log_built_shared}
  RELATIVE_INCLUDE_PATH "src"
  DEPENDENCIES ""
  #VERBOSE
)
