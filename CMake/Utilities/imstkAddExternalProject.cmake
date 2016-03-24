macro(imstk_add_external_project extProj)

  set(options VERBOSE)
  set(oneValueArgs REPOSITORY GIT_TAG)
  set(multiValueArgs CMAKE_ARGS DEPENDENCIES)
  include(CMakeParseArguments)
  cmake_parse_arguments(${extProj} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  #-----------------------------------------------------------------------------
  # Verbose (display arguments)
  #-----------------------------------------------------------------------------
  if(${extProj}_VERBOSE)
    foreach(opt ${options} ${oneValueArgs} ${multiValueArgs})
      message(STATUS "${extProj}_${opt}:${${extProj}_${opt}}")
    endforeach()
  endif()

  #-----------------------------------------------------------------------------
  # Sanity checks
  #-----------------------------------------------------------------------------
  if(DEFINED ${extProj}_DIR AND NOT EXISTS ${${extProj}_DIR})
    message(FATAL_ERROR
      "${extProj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  #-----------------------------------------------------------------------------
  # Solve dependencies
  #-----------------------------------------------------------------------------
  set(PROJECT_VAR_TEMP ${extProj})
  ExternalProject_Include_Dependencies( ${extProj}
    PROJECT_VAR PROJECT_VAR_TEMP
    DEPENDS_VAR ${extProj}_DEPENDENCIES
    USE_SYSTEM_VAR USE_SYSTEM_${extProj}
    SUPERBUILD_VAR ${PROJECT_NAME}_SUPERBUILD
    )

  #-----------------------------------------------------------------------------
  # If needs to download and build
  #-----------------------------------------------------------------------------
  if(NOT DEFINED ${extProj}_DIR AND NOT ${USE_SYSTEM_${extProj}})

    #-----------------------------------------------------------------------------
    # Set project directory
    #-----------------------------------------------------------------------------
    set(${extProj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/External/${extProj}/src)
    set(${extProj}_PREFIX_DIR ${CMAKE_BINARY_DIR}/External/${extProj}/cmake)
    set(${extProj}_DIR ${CMAKE_BINARY_DIR}/External/${extProj}/build-${CMAKE_BUILD_TYPE})
    set(${extProj}_INCLUDE_PATH ${${extProj}_DIR}/include)
    set(${extProj}_LIBRARY_PATH ${${extProj}_DIR}/library)

    #-----------------------------------------------------------------------------
    # Add project
    #-----------------------------------------------------------------------------
    ExternalProject_add( ${extProj}
      SOURCE_DIR ${${extProj}_SOURCE_DIR}
      BINARY_DIR ${${extProj}_DIR}
      PREFIX ${${extProj}_PREFIX_DIR}
      GIT_REPOSITORY ${${extProj}_REPOSITORY}
      GIT_TAG ${${extProj}_GIT_TAG}
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${CMAKE_GENERATOR}
      CMAKE_ARGS
        ${CMAKE_CONFIG_ARGS}
        ${CMAKE_CONFIG_OSX_ARGS}
        ${${extProj}_CMAKE_ARGS}
        -DCMAKE_INCLUDE_PATH:STRING=${${extProj}_INCLUDE_PATH}
        -DCMAKE_LIBRARY_PATH:STRING=${${extProj}_LIBRARY_PATH}
        #-DBUILD_SHARED_LIBS:BOOL=${shared}
        #-DBUILD_EXAMPLES:BOOL=OFF
        #-DBUILD_TESTING:BOOL=OFF
      DEPENDS ${${extProj}_DEPENDENCIES}
      )

  #-----------------------------------------------------------------------------
  # If project already built on system
  #-----------------------------------------------------------------------------
  else()

    if( ${USE_SYSTEM_${extProj}} )
      find_package( ${extProj} REQUIRED )
    endif()

    #-----------------------------------------------------------------------------
    # Add empty project (to solve dependencies)
    #-----------------------------------------------------------------------------
    ExternalProject_Add(${project_name}
      SOURCE_DIR ${${extProj}_SOURCE_DIR}
      BINARY_DIR ${${extProj}_DIR}
      PREFIX ${${extProj}_PREFIX_DIR}
      DOWNLOAD_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS ${${extProj}_DEPENDENCIES}
      )

  endif()

  #-----------------------------------------------------------------------------
  # Keep track of project path to add it in the superbuild cache
  #-----------------------------------------------------------------------------
  list( APPEND ${PROJECT_NAME}_EXTERNAL_PROJECTS_PATHS
    -D${extProj}_SOURCE_DIR:PATH=${${extProj}_SOURCE_DIR}
    -D${extProj}_DIR:PATH=${${extProj}_DIR}
    )
endmacro()
