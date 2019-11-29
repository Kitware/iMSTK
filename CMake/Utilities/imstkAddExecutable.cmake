
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
  add_custom_command(TARGET ${target} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${iMSTK_INSTALL_BIN_DIR})
endmacro()
