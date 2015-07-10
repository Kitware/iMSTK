

enable_testing()
include(CTest)
set(CPP_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if (APPLE)
  list(APPEND SimMedTK_TEST_ENV "DYLD_FRAMEWORK_PATH=${SimMedTK_BINARY_DIR}/../SuperBuild/SFML/extlibs/libs-osx/Frameworks")
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
      ENVIRONMENT "${SimMedTK_TEST_ENV}"
  )
endmacro()

#! \brief Add ctest test with data as input.
macro(simple_test_with_data TEST_NAME)
  simple_test(${TEST_NAME} ${ARGN} -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
  set_tests_properties(${TEST_NAME}
    PROPERTIES
      ENVIRONMENT "${SimMedTK_TEST_ENV}"
  )
endmacro()
