macro(imstk_subdir_list result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()


function(imstk_add_library target)

  set(options VERBOSE)
  set(oneValueArgs)
  set(multiValueArgs H_FILES CPP_FILES SUBDIR_LIST DEPENDS)
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
  # Get files and directories
  #-----------------------------------------------------------------------------
  if( NOT target_H_FILES AND NOT target_CPP_FILES )
    file(GLOB_RECURSE target_H_FILES "${CMAKE_CURRENT_SOURCE_DIR}/imstk*.h")
    file(GLOB_RECURSE target_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/imstk*.cpp")
    file(GLOB_RECURSE testing_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*")
    if(testing_FILES)
      list(REMOVE_ITEM target_H_FILES ${testing_FILES})
      list(REMOVE_ITEM target_CPP_FILES ${testing_FILES})
    endif()
  endif()

  if( NOT target_SUBDIR_LIST )
    imstk_subdir_list(target_SUBDIR_LIST ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  list(APPEND target_BUILD_INTERFACE_LIST "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
  foreach(subdir ${target_SUBDIR_LIST})
    if( NOT ${subdir} STREQUAL "Testing")
      list(APPEND target_BUILD_INTERFACE_LIST "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${subdir}>")
    endif()
  endforeach()

  #-----------------------------------------------------------------------------
  # Create target (library)
  #-----------------------------------------------------------------------------
  add_library( ${target} STATIC
    ${target_H_FILES}
    ${target_CPP_FILES}
    )

  #-----------------------------------------------------------------------------
  # Link libraries to current target
  #-----------------------------------------------------------------------------
  # Add dependent targets
  foreach(d ${target_DEPENDS})
    list(APPEND ${target}_LIBRARIES "${d}")
  endforeach()
  #message(STATUS "${target} using libraries : ${${target}_LIBRARIES}")
  target_link_libraries( ${target}
    ${${target}_LIBRARIES}
    )

  #-----------------------------------------------------------------------------
  # Include directories
  #-----------------------------------------------------------------------------
  target_include_directories( ${target} PUBLIC
    ${target_BUILD_INTERFACE_LIST}
    $<INSTALL_INTERFACE:${iMSTK_INSTALL_INCLUDE_DIR}>
    )

  #-----------------------------------------------------------------------------
  # Set compile flags for the target
  #-----------------------------------------------------------------------------  
  target_compile_options(${target} PRIVATE
                           $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                                -Wall>
                           $<$<CXX_COMPILER_ID:MSVC>:
                                -W4>)

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
	
	

  #-----------------------------------------------------------------------------
  # Add the target to imstk folder
  #-----------------------------------------------------------------------------
  SET_TARGET_PROPERTIES (${target} PROPERTIES FOLDER iMSTK)

endfunction()
