###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

find_package(Bandit REQUIRED)
include_directories(${BANDIT_INCLUDE_DIR})

include(CTest)
enable_testing()

set(CPP_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if (APPLE)
  list(APPEND iMSTK_TEST_ENV "DYLD_FRAMEWORK_PATH=${iMSTK_BINARY_DIR}/../SuperBuild/SFML/extlibs/libs-osx/Frameworks")
endif()

# Configuration for the CMake-generated test driver
set(CMAKE_TESTDRIVER_TESTMAIN "#include <bandit/bandit.h>

#if defined(WIN32)
#  pragma warning(disable : 4996)
#endif

using namespace bandit;

int main(int argc, char* argv[])
{
  return bandit::run(argc, argv);
}
")

set(BANDIT_RUNNER "${CMAKE_BINARY_DIR}/Testing/BanditRunner.cpp")
file(WRITE ${BANDIT_RUNNER} "${CMAKE_TESTDRIVER_TESTMAIN}")

#! \brief Add ctest test.
macro(simple_test TEST_NAME)
  add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${Module}UnitTestRunner> ${ARGN})
  set_tests_properties(${TEST_NAME}
    PROPERTIES
      ENVIRONMENT "${iMSTK_TEST_ENV}"
  )
endmacro()

#! \brief Add ctest test with data as input.
macro(simple_test_with_data TEST_NAME)
  simple_test(${TEST_NAME} ${ARGN} -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
  set_tests_properties(${TEST_NAME}
    PROPERTIES
      ENVIRONMENT "${iMSTK_TEST_ENV}"
  )
endmacro()
