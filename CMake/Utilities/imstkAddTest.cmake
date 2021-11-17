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
  file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/*Test.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/*Test.cpp")

  # Create test driver executable
  imstk_add_executable(${test_driver_executable} ${test_files})

  # Link test driver against current target and GTest
  target_link_libraries(${test_driver_executable}
    ${target}
    Testing
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

#.rst:
# .. cmake:command:: imstk_add_project_test
#
# .. code-block:: cmake
#
#  imstk_add_project_test(<test_name>
#      SOURCE_DIR <source_dir>
#      BINARY_DIR <binary_dir>
#      [WORKING_DIR <working_dir>]
#      [BUILD_OPTIONS <opt1> [...]]
#    )
#
#
# .. code-block:: cmake
#
#  SOURCE_DIR Source directory of the project to build
#
#  BINARY_DIR Binary directory of the project to build
#
#  WORKING_DIR Specify the working directory in which to execute the test. If not specified
#              the test will be run with the current working directory set to `<binary_dir>`.
#
#  BUILD_OPTIONS List of options for configuring the project.
#
function(imstk_add_project_test name)
  set(options)
  set(oneValueArgs SOURCE_DIR BINARY_DIR WORKING_DIR)
  set(multiValueArgs BUILD_OPTIONS)
  cmake_parse_arguments(_iapt "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  # Sanity checks
  set(expected_nonempty_vars SOURCE_DIR BINARY_DIR)
  foreach(var ${expected_nonempty_vars})
    if("${_iapt_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} argument is either empty or not specified")
    endif()
  endforeach()

  if(NOT _iapt_WORKING_DIR)
    set(_iapt_WORKING_DIR ${_iapt_BINARY_DIR})
  endif()

  if(_iapt_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to imstk_add_project_test(): \"${_iapt_UNPARSED_ARGUMENTS}\"")
  endif()

  set(_testname ${name})
  set(_test_source_dir ${_iapt_SOURCE_DIR})
  set(_test_binary_dir ${_iapt_BINARY_DIR})

  add_test(${_testname}_clean ${CMAKE_COMMAND}
    -E remove_directory ${_test_binary_dir}
    )

  set(build_generator_args
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    )
  if(CMAKE_GENERATOR_PLATFORM)
    list(APPEND build_generator_args
      --build-generator-platform ${CMAKE_GENERATOR_PLATFORM}
      )
  endif()
  if(CMAKE_GENERATOR_TOOLSET)
    list(APPEND build_generator_args
      --build-generator-toolset ${CMAKE_GENERATOR_TOOLSET}
      )
  endif()

  set(build_options_arg)
  if(_iapt_BUILD_OPTIONS)
    set(build_options_arg --build-options ${_iapt_BUILD_OPTIONS})
  endif()

  add_test(
    NAME ${_testname}
    COMMAND ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${_test_source_dir}"
      "${_test_binary_dir}"
      --build-config Release
      ${build_generator_args}
      --build-project ${project}
      --build-exe-dir "${_test_binary_dir}"
      --force-new-ctest-process
      ${build_options_arg}
      --test-command ${CMAKE_CTEST_COMMAND} -V
    WORKING_DIRECTORY ${_iapt_WORKING_DIR}
    )

  set_tests_properties(${_testname}
      PROPERTIES
        DEPENDS ${_testname}_clean
        LABELS "BuildSystem"
      )
endfunction()

