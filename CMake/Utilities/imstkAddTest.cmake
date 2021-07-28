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
include(GoogleTest)



function(imstk_add_test_internal target kind)

  set(test_driver_executable "${target}${kind}")
  
  # Get all source files
  file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/*Test.cpp")

  # Get all source file names
  set(test_file_names "")
  foreach(test_file ${test_files})
    get_filename_component(test_file_name ${test_file} NAME)
    list(APPEND test_file_names ${test_file_name})
  endforeach()

  # Create test driver executable
  imstk_add_executable(${test_driver_executable} ${test_files})

  # Link test driver against current target, gtest and pthread
  target_link_libraries(${test_driver_executable}
    ${target}
    Testing
    Threads::Threads
    GTest::gtest
    GTest::gmock

  )

if (MSVC)
  gtest_discover_tests(${test_driver_executable} WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin" DISCOVERY_MODE PRE_TEST)
else()
  gtest_add_tests(${test_driver_executable} SOURCES "${test_files}")
endif()

  #-----------------------------------------------------------------------------
  # Add the target to Testing folder
  #-----------------------------------------------------------------------------
  SET_TARGET_PROPERTIES (${test_driver_executable} PROPERTIES FOLDER Testing)

endfunction()

function(imstk_add_test target)
  imstk_add_test_internal(${target} Tests)
endfunction()

function(imstk_add_visual_test target)
  imstk_add_test_internal(${target} VisualTests)
endfunction()
