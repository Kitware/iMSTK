
set(extProj Uncrustify)
set(${extProj}_DEPENDENCIES "")
set(${extProj}_REPOSITORY ${git_protocol}://github.com/uncrustify/uncrustify.git)
set(${extProj}_GIT_TAG uncrustify-0.62)

#-----------------------------------------------------------------------------
# Sanity checks
#-----------------------------------------------------------------------------
if(DEFINED ${extProj}_EXECUTABLE AND NOT EXISTS ${${extProj}_EXECUTABLE})
  message(FATAL_ERROR
    "${extProj}_EXECUTABLE variable is defined but corresponds to non-existing executable")
endif()

#-----------------------------------------------------------------------------
# Solve dependencies
#-----------------------------------------------------------------------------
ExternalProject_Include_Dependencies( ${extProj}
  PROJECT_VAR extProj
  DEPENDS_VAR ${extProj}_DEPENDENCIES
  USE_SYSTEM_VAR USE_SYSTEM_${extProj}
  SUPERBUILD_VAR ${PROJECT_NAME}_SUPERBUILD
  )

#-----------------------------------------------------------------------------
# Set extProject directory
#-----------------------------------------------------------------------------
set(${extProj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/External/${extProj}/src)
set(${extProj}_PREFIX_DIR ${CMAKE_BINARY_DIR}/External/${extProj}/prefix)
set(${extProj}_DIR ${CMAKE_BINARY_DIR}/External/${extProj}/build)

#-----------------------------------------------------------------------------
# If needs to download and build
#-----------------------------------------------------------------------------
if(NOT DEFINED ${extProj}_EXECUTABLE AND NOT ${USE_SYSTEM_${extProj}})

  set(${extProj}_EXECUTABLE ${${extProj}_DIR}/src/uncrustify)

  #-----------------------------------------------------------------------------
  # Add extProject
  #-----------------------------------------------------------------------------
  ExternalProject_add(${extProj}
    GIT_REPOSITORY ${${extProj}_REPOSITORY}
    GIT_TAG ${${extProj}_GIT_TAG}
    SOURCE_DIR ${${extProj}_SOURCE_DIR}
    BINARY_DIR ${${extProj}_DIR}
    PREFIX ${${extProj}_PREFIX_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<BINARY_DIR> #TODO: does not work on Windows, see --host= option
    DEPENDS ${${extProj}_DEPENDENCIES}
    )

#-----------------------------------------------------------------------------
# If using system
#-----------------------------------------------------------------------------
elseif(${USE_SYSTEM_${extProj}})

  find_program(${extProj}_EXECUTABLE ${extProj} DOC "Path of ${extProj} program")
  if(NOT ${extProj}_EXECUTABLE)
    message(FATAL_ERROR "To use the system ${extProj}, set ${extProj}_EXECUTABLE")
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

set(TEST true)
#-----------------------------------------------------------------------------
# Keep track of project path to add it in the superbuild cache
#-----------------------------------------------------------------------------
list( APPEND ${PROJECT_NAME}_EXTERNAL_PROJECTS_PATHS -D${extProj}_EXECUTABLE:PATH=${${extProj}_EXECUTABLE} )
