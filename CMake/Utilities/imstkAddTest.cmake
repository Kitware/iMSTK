# imstk_add_test
#
# Description: Will create tests for the given iMSTK target.
#
# Requirements:
# - Requires a 'Testing' subdirectory with cpp files for each class to test.
# - Google Test should be used to create unit tests, see:
#   https://github.com/google/googletest/blob/master/googletest/docs/Documentation.md
# - Test files should be constructed as below (here with imstkMyClassTest.cpp):
#
#   #include "gtest/gtest.h"
#
#   class imstkMyClassTest : public ::testing::Test
#   {
#   protected:
#       MyClass m_objectToTest;
#   };
#
#   TEST_F(imstkMyClassTest, TestName1)
#   {
#       # test body on m_objectToTest
#   }
#
#   TEST_F(imstkMyClassTest, TestName2)
#   {
#       # test body on m_objectToTest
#   }
#
#   int imstkMyClassTest(int argc, char* argv[])
#   {
#       // Init Google Test
#       ::testing::InitGoogleTest(&argc, argv);
#
#       // Run tests with gtest
#       return RUN_ALL_TESTS();
#   }
#
function(imstk_add_test target)

  set(test_driver_name "imstk${target}TestDriver")
  string(TOLOWER ${target} target_lowercase)
  set(test_driver_executable "imstk_${target_lowercase}_test_driver")
  message(STATUS "Configuring ${test_driver_executable}")

  #-----------------------------------------------------------------------------
  # Check that Testing repository exists
  #-----------------------------------------------------------------------------
  if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Testing")
    message(WARNING "Can not create test driver for ${target} target: missing 'Testing' directory in ${CMAKE_CURRENT_SOURCE_DIR}.")
    return()
  endif()

  #-----------------------------------------------------------------------------
  # Create test driver
  #-----------------------------------------------------------------------------
  # Get all source files
  file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*Test.cpp")

  # Get all source file names
  set(test_file_names "")
  foreach(test_file ${test_files})
    get_filename_component(test_file_name ${test_file} NAME)
    list(APPEND test_file_names ${test_file_name})
  endforeach()

  # Create test driver main source file
  create_test_sourcelist(test_sourcelist ${test_driver_name}.cpp ${test_file_names})

  # Create test driver executable
  add_executable(${test_driver_executable} ${test_driver_name}.cpp ${test_files})

  # Link test driver against current target, gtest and pthread
  target_link_libraries(${test_driver_executable}
    ${target}
    ${GoogleTest_LIBRARIES}
    Threads::Threads
  )

  #-----------------------------------------------------------------------------
  # Create tests
  #-----------------------------------------------------------------------------
  foreach(test_file ${test_files})
    get_filename_component(test_name ${test_file} NAME_WE)

    include(ExternalData)
    ExternalData_add_test( ${PROJECT_NAME}Data
      NAME ${test_name}
      COMMAND $<TARGET_FILE:${test_driver_executable}> ${test_name} --gtest_filter=${test_name}.*
    )

  endforeach()

endfunction()
