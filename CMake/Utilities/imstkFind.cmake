include(FindPackageHandleStandardArgs)
#-----------------------------------------------------------------------------
# Look for header directory
#-----------------------------------------------------------------------------

macro(imstk_find_header package header)

  # Subdir to the include directory is optional
  set(sub_dir "")
  set (extra_macro_args ${ARGN})
  # Did we get any optional args?
  list(LENGTH extra_macro_args num_extra_args)
  if (${num_extra_args} GREATER 0)
    list(GET extra_macro_args 0 sub_dir)
  endif ()

  unset(_SEARCH_DIR)
  if(${package}_ROOT_DIR)
    set(_SEARCH_DIR ${${package}_ROOT_DIR}/${sub_dir})
  else()
    set(_SEARCH_DIR ${CMAKE_INSTALL_PREFIX}/include/${sub_dir})
  endif()
  #message(STATUS "Searching for : ${_SEARCH_DIR}/${header}")

  find_path(${package}_INCLUDE_DIR
  NAMES
    ${header}
  PATHS
    ${_SEARCH_DIR}
  NO_DEFAULT_PATH
    )
  #message(STATUS "Found it : ${${package}_INCLUDE_DIR}/${header}")

  if (EXISTS ${${package}_INCLUDE_DIR}/${header})
    string(TOUPPER ${package} PACKAGE)
    list(APPEND ${PACKAGE}_INCLUDE_DIRS ${${package}_INCLUDE_DIR})
    mark_as_advanced(${PACKAGE}_INCLUDE_DIRS)
  else()
    message(FATAL_ERROR "Could not find ${${package}_INCLUDE_DIR}/${header}")
  endif()
  #message(STATUS "${package}_INCLUDE_DIRS : ${${package}_INCLUDE_DIRS}")
  unset(${package}_INCLUDE_DIR CACHE)
endmacro()

#-----------------------------------------------------------------------------
# Look for both optimized and debug external libraries
#-----------------------------------------------------------------------------

macro(imstk_find_libary package library)

  # Debug postfix argument is optional
  set(debug_postfix ${CMAKE_DEBUG_POSTFIX})
  set (extra_macro_args ${ARGN})
  # Did we get any optional args?
  list(LENGTH extra_macro_args num_extra_args)
  if (${num_extra_args} GREATER 0)
    list(GET extra_macro_args 0 debug_postfix)
    string(STRIP ${debug_postfix} debug_postfix)
    #message(STATUS "${package} changing debug_postfix to ${debug_postfix}")
  endif()
  
  unset(_SEARCH_DIR)
  if(${package}_ROOT_DIR)
    if(${package}_LIB_DIR)
      set(_SEARCH_DIR ${${package}_ROOT_DIR}/${${package}_LIB_DIR})
    endif()
    #message(STATUS "Looking for ${package} libs in ${_SEARCH_DIR}")
  else()
    set(_SEARCH_DIR ${CMAKE_INSTALL_PREFIX}/lib)
  endif()
  
  
  string(TOUPPER ${package} PACKAGE)
  find_library(${PACKAGE}_LIBRARY_${library}-RELEASE
      NAMES
        ${library}
        lib${library}
      PATHS
        ${_SEARCH_DIR}
        ${_SEARCH_DIR}/Release
      NO_DEFAULT_PATH
  )
  if (EXISTS ${${PACKAGE}_LIBRARY_${library}-RELEASE})
    #message(STATUS "${PACKAGE}_LIBRARY_${library}-RELEASE : ${${PACKAGE}_LIBRARY_${library}-RELEASE}")
    list(APPEND ${PACKAGE}_LIBRARIES optimized ${${PACKAGE}_LIBRARY_${library}-RELEASE})
    list(APPEND ${PACKAGE}_RELEASE_LIBRARIES ${${PACKAGE}_LIBRARY_${library}-RELEASE})
  endif()
  mark_as_advanced(${PACKAGE}_LIBRARY_${library}-RELEASE)
  #message(STATUS "Libraries : ${${PACKAGE}_RELEASE_LIBRARIES}")
  
  find_library(${PACKAGE}_LIBRARY_${library}-DEBUG
      NAMES
        ${library}${debug_postfix}
        lib${library}${debug_postfix}
      PATHS
        ${_SEARCH_DIR}
        ${_SEARCH_DIR}/Debug
      NO_DEFAULT_PATH
  )
  if (EXISTS ${${PACKAGE}_LIBRARY_${library}-DEBUG})
    #message(STATUS "${PACKAGE}_LIBRARY_${library}-DEBUG : ${${PACKAGE}_LIBRARY_${library}-DEBUG}")
    list(APPEND ${PACKAGE}_LIBRARIES debug ${${PACKAGE}_LIBRARY_${library}-DEBUG})
    list(APPEND ${PACKAGE}_DEBUG_LIBRARIES ${${PACKAGE}_LIBRARY_${library}-DEBUG})
  endif()
  mark_as_advanced(${PACKAGE}_LIBRARY_${library}-DEBUG)
  #message(STATUS "Libraries : ${${PACKAGE}_DEBUG_LIBRARIES}")
endmacro()

#-----------------------------------------------------------------------------
# Confirm the package was found and was complete
#-----------------------------------------------------------------------------
macro(imstk_find_package package)

  string(TOUPPER ${package} PACKAGE)
  find_package_handle_standard_args(${PACKAGE}
    REQUIRED_VARS
      ${PACKAGE}_INCLUDE_DIRS
      ${PACKAGE}_LIBRARIES)

  # custom target package name is optional
  set(tgt_name ${package})
  set (extra_macro_args ${ARGN})
  # Did we get any optional args?
  list(LENGTH extra_macro_args num_extra_args)
  if (${num_extra_args} GREATER 0)
    list(GET extra_macro_args 0 tgt_name)
  endif ()

  #if(${PACKAGE}_FOUND AND NOT TARGET ${tgt_name})
    add_library(${tgt_name} INTERFACE IMPORTED)
    target_include_directories(${tgt_name} INTERFACE "${${PACKAGE}_INCLUDE_DIRS}")
    foreach(lib ${${PACKAGE}_DEBUG_LIBRARIES})
      target_link_libraries(${tgt_name} INTERFACE debug ${lib})
    endforeach()
    foreach(lib ${${PACKAGE}_RELEASE_LIBRARIES})
      target_link_libraries(${tgt_name} INTERFACE optimized ${lib})
    endforeach()
  #endif()
  #message(STATUS "Target ${tgt_name} libs : ${${PACKAGE}_LIBRARIES}")
endmacro()

#-----------------------------------------------------------------------------
# Confirm the (header only) package was found and was complete
#-----------------------------------------------------------------------------
macro(imstk_find_header_package package)

  string(TOUPPER ${package} PACKAGE)
  find_package_handle_standard_args(${PACKAGE}
    REQUIRED_VARS
      ${PACKAGE}_INCLUDE_DIRS)

  # custom target package name is optional
  set(tgt_name ${package})
  set (extra_macro_args ${ARGN})
  # Did we get any optional args?
  list(LENGTH extra_macro_args num_extra_args)
  if (${num_extra_args} GREATER 0)
    list(GET extra_macro_args 0 tgt_name)
  endif ()

  if(${PACKAGE}_FOUND AND NOT TARGET ${tgt_name})
    add_library(${tgt_name} INTERFACE IMPORTED)
    target_include_directories(${tgt_name} INTERFACE "${${PACKAGE}_INCLUDE_DIRS}")
  endif()
endmacro()