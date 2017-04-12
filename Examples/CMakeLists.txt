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

#-----------------------------------------------------------------------------
# Add Sandbox subdirectory
#-----------------------------------------------------------------------------
add_subdirectory(Sandbox)
