#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

if(MSVC)
  set(g3log_built_library ${g3log_BINARY_DIR}/$(Configuration)/g3logger$<$<CONFIG:Debug>:d>.lib)
else()
  set(g3log_built_library ${g3log_BINARY_DIR}/libg3logger.a)
endif()

include(imstkAddExternalProject)
imstk_define_external_dirs( g3log )
imstk_add_external_project( g3log
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/g3log.git
  GIT_TAG 6c1698c4f7db6b9e4246ead38051f9866ea3ac06
  CMAKE_CACHE_ARGS
    -DADD_FATAL_EXAMPLE:BOOL=OFF
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${g3log_SOURCE_DIR}/src/g3log ${CMAKE_INSTALL_PREFIX}/include/g3log
    COMMAND ${CMAKE_COMMAND} -E copy ${g3log_built_library} ${CMAKE_INSTALL_PREFIX}/lib/
  RELATIVE_INCLUDE_PATH "src"
  DEPENDENCIES ""
  #VERBOSE
  )
