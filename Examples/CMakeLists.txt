#-----------------------------------------------------------------------------
# Add ExternalData
#-----------------------------------------------------------------------------
# Remove the data if already here
set(hashStampPath "${ExternalData_BINARY_ROOT}/Data/Testing/ExternalDataTest.txt-hash-stamp")
if(EXISTS hashStampPath)
  file(READ hashStampPath hash)
  string(STRIP "${hash}" hash)
endif()
execute_process(
    COMMAND rm ${ExternalData_BINARY_ROOT}/Data/Testing/ExternalDataTest.txt
    COMMAND rm ${ExternalData_BINARY_ROOT}/Data/Testing/ExternalDataTest.txt-hash-stamp
    COMMAND rm ${ExternalData_BINARY_ROOT}/Objects/SHA512/${hash}
    OUTPUT_QUIET
    ERROR_QUIET)

# Download the data
imstk_add_data(ExternalDataTest "Testing/ExternalDataTest.txt")

#-----------------------------------------------------------------------------
# Add Test
#-----------------------------------------------------------------------------
# Check if the data has been correctly downloaded
add_test(
  NAME imstkExternalDataTest
  COMMAND ${CMAKE_COMMAND}
  -DFileToCheck=${ExternalData_BINARY_ROOT}/Data/Testing/ExternalDataTest.txt
  -P ${CMAKE_SOURCE_DIR}/CMake/Utilities/imstkCheckFileExists.cmake
)

# Associate external data needed for testing
list(APPEND FILE_LIST
    asianDragon/,REGEX:.*,
    human/,REGEX:.*,
    oneTet/,REGEX:.*)

#-----------------------------------------------------------------------------
# Add Example subdirectories
#-----------------------------------------------------------------------------
if(BUILD_EXAMPLES)
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
endif()

#-----------------------------------------------------------------------------
# Add external data
#-----------------------------------------------------------------------------
imstk_add_data(${PROJECT_NAME} ${FILE_LIST})
