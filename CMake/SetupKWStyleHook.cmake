#-----------------------------------------------------------------------------
# Define configuration files
#-----------------------------------------------------------------------------
set(kwstyle_configuration_file "Utilities/KWStyle/${PROJECT_NAME}.kws.xml")
set(kwstyle_overwrite_file "Utilities/KWStyle/KWStyleOverwrite.txt")

#-----------------------------------------------------------------------------
# Setup git hook for KWStyle executable path
#-----------------------------------------------------------------------------
find_package(Git)
if(GIT_FOUND)
  if(EXISTS "${KWSTYLE_EXECUTABLE}")
    message(STATUS "KWStyle found: enabling in git hook")
    execute_process(COMMAND ${GIT_EXECUTABLE} config kws.path "${KWSTYLE_EXECUTABLE}"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    execute_process(COMMAND ${GIT_EXECUTABLE} config kws.conf "${kwstyle_configuration_file}"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    execute_process(COMMAND ${GIT_EXECUTABLE} config kws.overwriteRulesConf "${kwstyle_overwrite_file}"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    execute_process( COMMAND ${GIT_EXECUTABLE} config kws.enabled true
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
  else()
    message(STATUS "KWStyle not found: disabling in git hook")
    execute_process(COMMAND ${GIT_EXECUTABLE} config --unset kws.path
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    execute_process(COMMAND ${GIT_EXECUTABLE} config --unset kws.conf
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    execute_process(COMMAND ${GIT_EXECUTABLE} config --unset kws.overwriteRulesConf
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    execute_process(COMMAND ${GIT_EXECUTABLE} config kws.enabled false
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
  endif()
endif()