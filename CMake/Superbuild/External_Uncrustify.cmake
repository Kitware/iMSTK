set(proj Uncrustify)
string(TOUPPER "${proj}" PROJ)

#-----------------------------------------------------------------------------
# Make sure this file is included only once
#-----------------------------------------------------------------------------
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

#-----------------------------------------------------------------------------
# Sanity checks
#-----------------------------------------------------------------------------
if(DEFINED ${PROJ}_EXECUTABLE AND NOT EXISTS ${${PROJ}_EXECUTABLE})
  message(FATAL_ERROR "${PROJ}_EXECUTABLE variable is defined but corresponds to non-existing file")
endif()

#-----------------------------------------------------------------------------
# Set dependency list
#-----------------------------------------------------------------------------
set(${proj}_DEPENDENCIES "")

#-----------------------------------------------------------------------------
# If needs to download and build
#-----------------------------------------------------------------------------
if(NOT DEFINED ${PROJ}_EXECUTABLE AND NOT ${USE_SYSTEM_${PROJ}})

  #-----------------------------------------------------------------------------
  # Set project Git info
  #-----------------------------------------------------------------------------
  set(${proj}_REPOSITORY ${git_protocol}://github.com/uncrustify/uncrustify.git)
  set(${proj}_GIT_TAG cf7bbac22c65df32dea3f0abf949cba80fc66d00)
  
  #-----------------------------------------------------------------------------
  # Set project directory
  #-----------------------------------------------------------------------------
  set(${proj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/Superbuild/${proj})
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/Superbuild/${proj}-build)

  #-----------------------------------------------------------------------------
  # Add project
  #-----------------------------------------------------------------------------
  include(ExternalProject)
  ExternalProject_add(${proj}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_GIT_TAG}
    UPDATE_COMMAND ""
    SOURCE_DIR ${${proj}_SOURCE_DIR}
    BINARY_DIR ${${proj}_DIR}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${${proj}_DIR}
    DEPENDS ${${proj}_DEPENDENCIES}
    )
  set(${PROJ}_EXECUTABLE ${${proj}_DIR}/bin/uncrustify)

#-----------------------------------------------------------------------------
# If using system
#-----------------------------------------------------------------------------
else()
  if(${USE_SYSTEM_${PROJ}})
    find_program(${PROJ}_EXECUTABLE ${proj} DOC "Path of ${proj} program")
    if(NOT ${PROJ}_EXECUTABLE)
      message(FATAL_ERROR "To use the system ${proj}, set ${PROJ}_EXECUTABLE")
    endif()
  endif()
endif()
