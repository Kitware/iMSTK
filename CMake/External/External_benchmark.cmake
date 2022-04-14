#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)

if(NOT DEFINED iMSTK_benchmark_GIT_SHA)
  set(iMSTK_benchmark_GIT_SHA "0d98dba29d66e93259db7daa53a9327df767a415")
endif()
if(NOT DEFINED iMSTK_benchmark_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://github.com/google/benchmark/archive/${iMSTK_benchmark_GIT_SHA}.zip
    URL_HASH MD5=3f38f0497a19f663f82f9d766aa574e1
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_benchmark_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_benchmark_GIT_SHA}
    )
endif()

imstk_add_external_project( benchmark
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  CMAKE_CACHE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -Dgtest_force_shared_crt:BOOL=ON
    -DBENCHMARK_DOWNLOAD_DEPENDENCIES:BOOL=ON
    -DBENCHMARK_ENABLE_GTEST_TESTS:BOOL=OFF
    -DBENCHMARK_ENABLE_TESTING:BOOL=OFF
  DEPENDENCIES ""
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )

if(NOT USE_SYSTEM_benchmark)
  set(benchmark_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/benchmark)
  #message(STATUS "benchmark_DIR : ${benchmark_DIR}")
endif()