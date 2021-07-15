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
  set(multiValueArgs H_FILES CPP_FILES SUBDIR_LIST EXCLUDE_FILES DEPENDS)
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

  # Exclude files
  foreach(file ${target_EXCLUDE_FILES})
    list(REMOVE_ITEM target_H_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    list(REMOVE_ITEM target_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    list(REMOVE_ITEM testing_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  endforeach()

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
  target_link_libraries( ${target} PUBLIC
    ${${target}_LIBRARIES}
    )

  #-----------------------------------------------------------------------------
  # Include directories
  #-----------------------------------------------------------------------------
  target_include_directories( ${target} PUBLIC
    ${target_BUILD_INTERFACE_LIST}
    $<INSTALL_INTERFACE:include/${${PROJECT_NAME}_INSTALL_FOLDER}>
    )

  #-----------------------------------------------------------------------------
  # Set compile flags for the target
  #-----------------------------------------------------------------------------  
  if (iMSTK_COLOR_OUTPUT)
    target_compile_options(${target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Wno-unused-function -fdiagnostics-color=always>
      $<$<CXX_COMPILER_ID:MSVC>:
          -W4 -MP -wd4505 /bigobj>)
  else()
    target_compile_options(${target} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wno-unused-function>
    $<$<CXX_COMPILER_ID:MSVC>:
        -W4 -MP -wd4505 /bigobj>)
  endif()

  #-----------------------------------------------------------------------------
  # Install headers
  #-----------------------------------------------------------------------------
  install( FILES
    ${target_H_FILES}
    DESTINATION include/${${PROJECT_NAME}_INSTALL_FOLDER}
    COMPONENT Development
    )

  #-----------------------------------------------------------------------------
  # Install library
  #-----------------------------------------------------------------------------
  install( TARGETS ${target} EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib COMPONENT Development
    )
	
	

  #-----------------------------------------------------------------------------
  # Add the target to imstk folder
  #-----------------------------------------------------------------------------
  SET_TARGET_PROPERTIES (${target} PROPERTIES FOLDER iMSTK)

endfunction()
