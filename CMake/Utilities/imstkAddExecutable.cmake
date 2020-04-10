
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
  set_target_properties(${target} PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
  add_custom_command(TARGET ${target} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${CMAKE_INSTALL_PREFIX}/bin)
                   
  #-----------------------------------------------------------------------------
  # Set MSVC working directory to the install/bin directory
  #-----------------------------------------------------------------------------
  if(MSVC) # Configure running executable out of MSVC
    set_property(TARGET ${target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")
  endif()
  
endmacro()
