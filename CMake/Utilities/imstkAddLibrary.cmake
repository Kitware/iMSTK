function(imstk_add_library target)

  set(options VERBOSE)
  set(oneValueArgs)
  set(multiValueArgs H_FILES CPP_FILES LIBRARIES)
  include(CMakeParseArguments)
  cmake_parse_arguments(target "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  message(STATUS "Configuring ${target}")

  #-----------------------------------------------------------------------------
  # Verbose (display arguments)
  #-----------------------------------------------------------------------------
  if(target_VERBOSE)
    foreach(opt ${options} ${oneValueArgs} ${multiValueArgs})
      message(STATUS "${opt}:${target_${opt}}")
    endforeach()
  endif()

  #-----------------------------------------------------------------------------
  # Create target (library)
  #-----------------------------------------------------------------------------
  add_library( ${target} STATIC
    ${target_H_FILES}
    ${target_CPP_FILES}
    )
  set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)

  #-----------------------------------------------------------------------------
  # Link libraries to current target
  #-----------------------------------------------------------------------------
  target_link_libraries( ${target}
    ${target_LIBRARIES}
    )

  #-----------------------------------------------------------------------------
  # Include directories
  #-----------------------------------------------------------------------------
  target_include_directories( ${target} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${iMSTK_INSTALL_INCLUDE_DIR}>
    )

  #-----------------------------------------------------------------------------
  # Install headers
  #-----------------------------------------------------------------------------
  install( FILES
    ${target_H_FILES}
    DESTINATION ${iMSTK_INSTALL_INCLUDE_DIR}
    COMPONENT Development
    )

  #-----------------------------------------------------------------------------
  # Install library
  #-----------------------------------------------------------------------------
  install( TARGETS ${target} EXPORT iMSTK_TARGETS
    RUNTIME DESTINATION ${iMSTK_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${iMSTK_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${iMSTK_INSTALL_LIB_DIR} COMPONENT Development
    )

endfunction()
