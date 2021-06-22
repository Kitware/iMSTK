
macro(imstk_add_executable target)
  set (files ${ARGN})
  list(LENGTH files num_files)
    if (${num_files} EQUAL 0)
        message ("No files associated with target ${target}")
    endif ()
  add_executable(${target} ${files})
  if (VTK_VERSION VERSION_GREATER_EQUAL  "8.90")
    vtk_module_autoinit(TARGETS ${target} MODULES ${VTK_LIBRARIES})
  endif()

  if (iMSTK_COLOR_OUTPUT)
    target_compile_options(${target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Wno-unused-function -fdiagnostics-color=always>
      $<$<CXX_COMPILER_ID:MSVC>:
          -W4 -MP -wd4505 /bigobj /permissive->)
  else()
    target_compile_options(${target} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wno-unused-function>
    $<$<CXX_COMPILER_ID:MSVC>:
        -W4 -MP -wd4505 /bigobj /permissive->)
  endif()

  set_target_properties(${target} PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

  # Copy to install directory
  add_custom_command(TARGET ${target} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${CMAKE_INSTALL_PREFIX}/bin)
                   
  #-----------------------------------------------------------------------------
  # Set MSVC working directory to the install/bin directory
  #-----------------------------------------------------------------------------
  if(MSVC) # Configure running executable out of MSVC
    set_property(TARGET ${target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")
  endif()
  
endmacro()
