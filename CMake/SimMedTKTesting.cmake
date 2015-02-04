if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  set(CPP_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  # Configuration for the CMake-generated test driver
  set(CMAKE_TESTDRIVER_EXTRA_INCLUDES "
#include <stdexcept>
#if defined(WIN32)
#  pragma warning(disable : 4996)
#endif")
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "
  try
    {")
  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "
    }
  catch( std::exception & excp )
    {
    fprintf(stderr, \"Test driver caught exception: [%s]\\n\", excp.what());
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    printf(\"Exception caught by the test driver\\n\");
    return EXIT_FAILURE;
    }")
endif()
