#-----------------------------------------------------------------------------
# Add Example subdirectories
#-----------------------------------------------------------------------------
macro(listOfSubDir result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

listOfSubDir(subDirs ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${subDirs})
  add_subdirectory(${subdir})
endforeach() 


