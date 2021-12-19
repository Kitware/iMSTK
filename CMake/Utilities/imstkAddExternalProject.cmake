#-----------------------------------------------------------------------------
#
# Macro: imstk_add_external_project
#
# Goal:  Wraps ExternalProject_Include_Dependencies and ExternalProject_add.
#
# Parsed arguments :
# - SOURCE_DIR : location of the external project sources. If not specified,
#   it defaults to "External/EXTERNAL_PROJECT_NAME/src"
# - RELATIVE_INCLUDE_PATH : path to the project include files relatively t
#   SOURCE_DIR (above). Used to populate the variable CMAKE_INCLUDE_PATH.
# - DEPENDENCIES : Name of dependencies needed by this project.
#
# Other arguments : any other arguments will be passed to ExternalProject_Add
# directly. Look at ExternalProject_Add documentation for more inputs info.
#
#-----------------------------------------------------------------------------

set(SKIP_STEP_COMMAND ${CMAKE_COMMAND} -E echo "Skip step")

macro(imstk_define_external_dirs extProj)
  #-----------------------------------------------------------------------------
  # Set project directory
  #-----------------------------------------------------------------------------
  set(${extProj}_PREFIX "${CMAKE_BINARY_DIR}/External/${extProj}")
  if("${${extProj}_SOURCE_DIR}" STREQUAL "")
    set(${extProj}_SOURCE_DIR "${${extProj}_PREFIX}/src")
  endif()
  if("${${extProj}_BINARY_DIR}" STREQUAL "")
    set(${extProj}_BINARY_DIR "${${extProj}_PREFIX}/build")
  endif()
  set(${extProj}_TMP_DIR "${${extProj}_PREFIX}/tmp")
  set(${extProj}_STAMP_DIR "${${extProj}_PREFIX}/stamp")
endmacro()

macro(imstk_add_external_project proj)

  #-----------------------------------------------------------------------------
  # Parse arguments
  #-----------------------------------------------------------------------------
  set(options VERBOSE)
  set(oneValueArgs RELATIVE_INCLUDE_PATH SOURCE_DIR BINARY_DIR)
  set(multiValueArgs DEPENDENCIES)
  include(CMakeParseArguments)
  cmake_parse_arguments(_imstk_add_ep_${proj} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  #-----------------------------------------------------------------------------
  # Verbose (display arguments)
  #-----------------------------------------------------------------------------
  if(_imstk_add_ep_${proj}_VERBOSE)
    foreach(opt ${options} ${oneValueArgs} ${multiValueArgs})
      message(STATUS "_imstk_add_ep_${proj}_${opt}: ${_imstk_add_ep_${proj}_${opt}}")
    endforeach()
    message(STATUS "_imstk_add_ep_${proj}_UNPARSED_ARGUMENTS: ${_imstk_add_ep_${proj}_UNPARSED_ARGUMENTS}")
  endif()

  #-----------------------------------------------------------------------------
  # Sanity checks
  #-----------------------------------------------------------------------------
  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR
      "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  #-----------------------------------------------------------------------------
  # Solve dependencies
  #-----------------------------------------------------------------------------
  set(proj ${proj})
  ExternalProject_Include_Dependencies( ${proj}
    PROJECT_VAR proj
    EP_ARGS_VAR ${proj}_EP_ARGS
    DEPENDS_VAR _imstk_add_ep_${proj}_DEPENDENCIES
    USE_SYSTEM_VAR USE_SYSTEM_${proj}
    SUPERBUILD_VAR ${PROJECT_NAME}_SUPERBUILD
    )

  #-----------------------------------------------------------------------------
  # If needs to download and build
  #-----------------------------------------------------------------------------
  if(NOT DEFINED ${proj}_DIR AND NOT USE_SYSTEM_${proj})

    imstk_define_external_dirs( ${proj} )

    # SOURCE_DIR and BINARY_DIR variables set in the caller scope or
    # by "imstk_define_external_dirs" take precedence
    if(DEFINED ${proj}_SOURCE_DIR)
      set(_imstk_add_ep_${proj}_SOURCE_DIR ${${proj}_SOURCE_DIR})
    endif()
    if(DEFINED ${proj}_BINARY_DIR)
      set(_imstk_add_ep_${proj}_BINARY_DIR ${${proj}_BINARY_DIR})
    endif()

    set(${proj}_CMAKE_CACHE_ARGS)
    if(DEFINED ${proj}_INSTALL_DIR)
      list(APPEND ${proj}_CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${${proj}_INSTALL_DIR}
        )
    endif()

    #-----------------------------------------------------------------------------
    # Add project
    #-----------------------------------------------------------------------------    
    ExternalProject_add( ${proj}
      PREFIX ${${proj}_PREFIX} # from caller's scope (see imstk_define_external_dirs)
      SOURCE_DIR ${_imstk_add_ep_${proj}_SOURCE_DIR} # from caller's scope (see imstk_define_external_dirs) or parsed argument
      BINARY_DIR ${_imstk_add_ep_${proj}_BINARY_DIR} # from caller's scope (see imstk_define_external_dirs) or parsed argument
      TMP_DIR ${${proj}_TMP_DIR}       # from caller's scope (see imstk_define_external_dirs)
      STAMP_DIR ${${proj}_STAMP_DIR}   # from caller's scope (see imstk_define_external_dirs)
      ${${proj}_EP_ARGS}               # from ExternalProject_Include_Dependencies
      CMAKE_CACHE_ARGS ${${proj}_CMAKE_CACHE_ARGS} # from above
      ${_imstk_add_ep_${proj}_UNPARSED_ARGUMENTS}    # from unparsed arguments of this macro
      DEPENDS ${_imstk_add_ep_${proj}_DEPENDENCIES}  # from parsed argument
      )
    
    #-----------------------------------------------------------------------------
    # Add the target to ExternalDeps folder
    #-----------------------------------------------------------------------------
    SET_TARGET_PROPERTIES (${proj} PROPERTIES FOLDER ExternalDeps)

  #-----------------------------------------------------------------------------
  # If project already built on system
  #-----------------------------------------------------------------------------
  else()

    #-----------------------------------------------------------------------------
    # Find package if USE_SYSTEM
    #-----------------------------------------------------------------------------
    if( ${USE_SYSTEM_${proj}} )
      find_package( ${proj} REQUIRED )
    endif()

    #-----------------------------------------------------------------------------
    # Add empty project (to solve dependencies)
    #-----------------------------------------------------------------------------
    ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})

  endif()

  #-----------------------------------------------------------------------------
  # Keep track of include path for superbuild
  #-----------------------------------------------------------------------------
  list(APPEND CMAKE_INCLUDE_PATH
     ${_imstk_add_ep_${proj}_SOURCE_DIR}/${_imstk_add_ep_${proj}_RELATIVE_INCLUDE_PATH}
     )

endmacro()
