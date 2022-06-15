set(CTEST_USE_LAUNCHERS "ON" CACHE STRING "")

# Something in iMSTK is not letting `CTEST_BUILD_CONFIGURATION` work. Just set
# it to something here instead.
set(build_type "Release")
if (NOT "$ENV{CMAKE_BUILD_TYPE}" STREQUAL "")
  set(build_type "$ENV{CMAKE_BUILD_TYPE}")
endif ()
set(CMAKE_BUILD_TYPE "${build_type}" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_sccache.cmake")
