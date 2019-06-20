# Add target to run uncrustify
  add_custom_target(uncrustifyRun
    COMMAND ${Uncrustify_EXECUTABLE}
      -c ${CMAKE_CURRENT_LIST_DIR}/../../Utilities/Uncrustify/iMSTKUncrustify.cfg
      -F ${CMAKE_CURRENT_BINARY_DIR}/Uncrustify.list
      --no-backup
    COMMENT "Run uncrustify - overwrites source files"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

  if(BUILD_TESTING)
    # Add test to check style using uncrustify
    add_test(NAME uncrustifyCheck
      COMMAND ${Uncrustify_EXECUTABLE}
      -c ${CMAKE_CURRENT_LIST_DIR}/../../Utilities/Uncrustify/iMSTKUncrustify.cfg
      -F ${CMAKE_CURRENT_BINARY_DIR}/Uncrustify.list
      --check
      COMMENT "Run uncrustify in check mode"
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
      )
  endif(BUILD_TESTING)

  # Write list of files on which to run uncrustify.
  # Explicitly write a zero byte file when no source files are specified.
  # Otherwise, configure_file() creates a file with a single blank line, and
  # uncrustify interprets the blank line as a file name.
  file(GLOB_RECURSE UNCRUSTIFY_SOURCE_FILES *.h *.cpp)
  set(_uncrustify_list_in ${CMAKE_CURRENT_LIST_DIR}/../../Utilities/Uncrustify/uncrustify.list.in)
  set(_uncrustify_list_out ${CMAKE_CURRENT_BINARY_DIR}/uncrustify.list)
  if(UNCRUSTIFY_SOURCE_FILES)
    list(SORT UNCRUSTIFY_SOURCE_FILES)
    string(REPLACE ";" "\n" UNCRUSTIFY_SOURCE_FILES "${UNCRUSTIFY_SOURCE_FILES}")
    configure_file("${_uncrustify_list_in}" "${_uncrustify_list_out}" @ONLY)
  else()
    file(WRITE "${_uncrustify_list_out}")
  endif()