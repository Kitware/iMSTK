#-----------------------------------------------------------------------------
# Define and configure configuration files
#-----------------------------------------------------------------------------
set(kwstyle_configuration_file "Utilities/KWStyle/${PROJECT_NAME}.kws.xml")
set(kwstyle_overwrite_file "Utilities/KWStyle/KWStyleOverwrite.txt")
set(kwstyle_examples_files_list_file "Utilities/KWStyle/${PROJECT_NAME}ExamplesFiles.txt")
configure_file(
  ${CMAKE_SOURCE_DIR}/${kwstyle_examples_files_list_file}.in
  ${CMAKE_BINARY_DIR}/${kwstyle_examples_files_list_file}
  )

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

#-----------------------------------------------------------------------------
# Define formatting options for error messages of build target output
#-----------------------------------------------------------------------------
option(KWSTYLE_USE_VIM_FORMAT
  "Set KWStyle to generate errors with a VIM-compatible format."
  OFF
  )
option(KWSTYLE_USE_GCC_FORMAT
  "Set KWStyle to generate errors with a GCC-compatible format."
  OFF
  )
option(KWSTYLE_USE_MSVC_FORMAT
  "Set KWStyle to generate errors with a VisualStudio-compatible format."
  ${MSVC_IDE} # default to TRUE only with a Visual Studio IDE
  )
mark_as_advanced(KWSTYLE_USE_VIM_FORMAT)
mark_as_advanced(KWSTYLE_USE_GCC_FORMAT)
mark_as_advanced(KWSTYLE_USE_MSVC_FORMAT)

#-----------------------------------------------------------------------------
# Define Editor format
#-----------------------------------------------------------------------------
set(kwstyle_editor_format "")
if(KWSTYLE_USE_VIM_FORMAT)
  list(APPEND kwstyle_editor_format -vim)
endif()
if(KWSTYLE_USE_GCC_FORMAT)
  list(APPEND kwstyle_editor_format -gcc)
endif()
if(KWSTYLE_USE_MSVC_FORMAT)
  list(APPEND kwstyle_editor_format -msvc)
endif()

#-----------------------------------------------------------------------------
# Check only one format is set to TRUE
#-----------------------------------------------------------------------------
list(LENGTH kwstyle_editor_format kwstyle_editor_format_length)
if(kwstyle_editor_format_length GREATER 1)
  message(FATAL_ERROR "At most, only one of KWSTYLE_USE_*_FORMAT can be set to TRUE.")
endif()

#-----------------------------------------------------------------------------
# Add build target for examples
#-----------------------------------------------------------------------------
add_custom_target(KWStyleCheckExamples
  COMMAND ${KWSTYLE_EXECUTABLE}
    -xml ${CMAKE_SOURCE_DIR}/${kwstyle_configuration_file}
    -o ${CMAKE_SOURCE_DIR}/${kwstyle_overwrite_file}
    -D ${CMAKE_BINARY_DIR}/${kwstyle_examples_files_list_file}
    -v
    ${kwstyle_editor_format}
  COMMENT
    "Examples Style Checker"
  WORKING_DIRECTORY
    ${CMAKE_SOURCE_DIR} # the paths in kwstyle_configuration_file are relative
  )
