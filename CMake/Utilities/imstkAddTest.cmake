# gtest_add_tests
#
# Taken from https://github.com/Kitware/CMake/blob/master/Modules/FindGTest.cmake
#
# Only changed the 'add_test' line to add 'extra_args' before the 'gtest_filter' flag.
# Doing so allows to use gtest with a ctest driver created after create_test_sourcelist,
# by providing the test file name as the first argument to the driver.
#
# Changes should become a part of CMake. Better improvements could be made to bring
# ctest and gtest together.
#
function(GTEST_ADD_TESTS executable extra_args)
    if(NOT ARGN)
        message(FATAL_ERROR "Missing ARGN: Read the documentation for GTEST_ADD_TESTS")
    endif()
    if(ARGN STREQUAL "AUTO")
        # obtain sources used for building that executable
        get_property(ARGN TARGET ${executable} PROPERTY SOURCES)
    endif()
    set(gtest_case_name_regex ".*\\( *([A-Za-z_0-9]+) *, *([A-Za-z_0-9]+) *\\).*")
    set(gtest_test_type_regex "(TYPED_TEST|TEST_?[FP]?)")
    foreach(source ${ARGN})
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${source})
        file(READ "${source}" contents)
        string(REGEX MATCHALL "${gtest_test_type_regex} *\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
        foreach(hit ${found_tests})
          string(REGEX MATCH "${gtest_test_type_regex}" test_type ${hit})

          # Parameterized tests have a different signature for the filter
          if("x${test_type}" STREQUAL "xTEST_P")
            string(REGEX REPLACE ${gtest_case_name_regex}  "*/\\1.\\2/*" test_name ${hit})
          elseif("x${test_type}" STREQUAL "xTEST_F" OR "x${test_type}" STREQUAL "xTEST")
            string(REGEX REPLACE ${gtest_case_name_regex} "\\1.\\2" test_name ${hit})
          elseif("x${test_type}" STREQUAL "xTYPED_TEST")
            string(REGEX REPLACE ${gtest_case_name_regex} "\\1/*.\\2" test_name ${hit})
          else()
            message(WARNING "Could not parse GTest ${hit} for adding to CTest.")
            continue()
          endif()
          add_test(NAME ${test_name} COMMAND ${executable} ${extra_args} --gtest_filter=${test_name}) # changed here
        endforeach()
    endforeach()
endfunction()


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
  imstk_add_executable(${test_driver_executable} ${test_driver_name}.cpp ${test_files})

  # Link test driver against current target, gtest and pthread
  target_link_libraries(${test_driver_executable}
    ${target}
    ${GOOGLETEST_LIBRARIES}
    Threads::Threads
  )
  
  #-----------------------------------------------------------------------------
  # Add the target to Testing folder
  #-----------------------------------------------------------------------------
  SET_TARGET_PROPERTIES (${test_driver_executable} PROPERTIES FOLDER Testing)

  #-----------------------------------------------------------------------------
  # Create tests
  #-----------------------------------------------------------------------------
  foreach(test_file ${test_files})
    get_filename_component(test_name ${test_file} NAME_WE)

    # A. Registers tests per gTests
    # [aka] a lot of tests - Google Testing standards
    gtest_add_tests(${test_driver_executable} ${test_name} ${test_file})

    # ... or ...

    # B. Registers tests per test files
    # [aka] less tests - CTest standards
#    include(ExternalData)
#    ExternalData_add_test( ${PROJECT_NAME}Data
#      NAME ${test_name}
#      COMMAND $<TARGET_FILE:${test_driver_executable}> ${test_name} --gtest_filter=${test_name}.*
#    )

  endforeach()

endfunction()
