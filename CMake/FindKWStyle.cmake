#-----------------------------------------------------------------------------
# Find KWStyle executable
#-----------------------------------------------------------------------------
find_program(
  KWSTYLE_EXECUTABLE
  NAMES KWStyle
  DOC "Path to the KWStyle executable"
  )
mark_as_advanced(KWSTYLE_EXECUTABLE)

#-----------------------------------------------------------------------------
# Check KWstyle version
#-----------------------------------------------------------------------------
if(KWSTYLE_EXECUTABLE)
  execute_process(
    COMMAND ${KWSTYLE_EXECUTABLE} -version
    OUTPUT_VARIABLE KWSTYLE_VERSION_STRING
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(KWSTYLE_VERSION_STRING)
    # string(REPLACE ..) fails if the input is an empty string
    string(REPLACE
      "Version: "
      ""
      KWSTYLE_VERSION_STRING
      ${KWSTYLE_VERSION_STRING}
      )
  else()
    # CMake's find_package_handle_standard_args has a bug where the
    # version empty string ("") is always acceptable
    set(KWSTYLE_VERSION_STRING "?")
  endif()
endif()

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  KWStyle
  REQUIRED_VARS KWSTYLE_EXECUTABLE
  VERSION_VAR KWSTYLE_VERSION_STRING
  )
