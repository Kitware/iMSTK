include(ExternalProject)
include(imstkSolveDependencies)

#-----------------------------------------------------------------------------
# Git protocol option
#-----------------------------------------------------------------------------
if(NOT GIT_EXECUTABLE)
  find_package(Git REQUIRED)
endif()

#-----------------------------------------------------------------------------
# CMake global args
#-----------------------------------------------------------------------------
if(MSVC)
  # Allow the MP flag to get set externally
  set(iMSTK_ENABLE_MULTI_PROCESS_BUILDS ON CACHE BOOL "Enable multi-process builds")
  set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
  set(iMSTK_NUM_BUILD_PROCESSES ${PROCESSOR_COUNT} CACHE STRING "The maximum number of processes for the /MP flag")
  if(${PROJECT}_ENABLE_MULTI_PROCESS_BUILDS)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} /MP${iMSTK_NUM_BUILD_PROCESSES}" CACHE STRING INTERNAL FORCE)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP${iMSTK_NUM_BUILD_PROCESSES}" CACHE STRING INTERNAL FORCE)
  endif()
endif()

mark_as_superbuild(
  VARS
    CMAKE_BUILD_TYPE
    CMAKE_DEBUG_POSTFIX
    CMAKE_INSTALL_PREFIX
    CMAKE_MODULE_PATH
    CMAKE_PREFIX_PATH
    CMAKE_C_COMPILER
    CMAKE_CXX_COMPILER
    CMAKE_C_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_EXE_LINKER_FLAGS
    CMAKE_SHARED_LINKER_FLAGS
    CMAKE_CXX_STANDARD
    CMAKE_CXX_STANDARD_REQUIRED
    THREADS_PREFER_PTHREAD_FLAG
  ALL_PROJECTS
  )

#-----------------------------------------------------------------------------
# CMake args if Apple
#-----------------------------------------------------------------------------
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  mark_as_superbuild(
    VARS
      CMAKE_OSX_ARCHITECTURES
      CMAKE_OSX_SYSROOT
      CMAKE_OSX_DEPLOYMENT_TARGET
      CMAKE_MACOSX_RPATH
    ALL_PROJECTS
    )
endif()

#-----------------------------------------------------------------------------
# Keep track of include path for superbuild
#-----------------------------------------------------------------------------
set(CMAKE_INCLUDE_PATH )
mark_as_superbuild(VARS CMAKE_INCLUDE_PATH ALL_PROJECTS)

#-----------------------------------------------------------------------------
# Solve dependencies
#-----------------------------------------------------------------------------
set(EXTERNAL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}) # Location of the "External_*" files

ExternalProject_Include_Dependencies( ${PROJECT_NAME}
  DEPENDS_VAR ${PROJECT_NAME}_DEPENDENCIES
  EP_ARGS_VAR ${PROJECT_NAME}_EP_ARGS
  SUPERBUILD_VAR ${PROJECT_NAME}_SUPERBUILD
  )

set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
if (${PROJECT_NAME}_WRAP_CSHARP)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DSWIG_DIR:PATH=${SWIG_DIR}
    -DSWIG_EXECUTABLE:PATH=${SWIG_EXECUTABLE})
endif()

#-----------------------------------------------------------------------------
# Inner build of the main project
#-----------------------------------------------------------------------------
ExternalProject_Add( ${PROJECT_NAME}
  ${${PROJECT_NAME}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/Innerbuild
  CMAKE_CACHE_ARGS
    -D${PROJECT_NAME}_SUPERBUILD:BOOL=OFF
    # External Libraries
    -DEigen3_DIR:PATH=${Eigen3_DIR}    
    -DGTest_DIR:PATH=${GTest_DIR}
    -Dbenchmark_DIR:PATH=${benchmark_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
  DEPENDS ${${PROJECT_NAME}_DEPENDENCIES}
  )

ExternalProject_AlwaysConfigure(${PROJECT_NAME})

#-----------------------------------------------------------------------------
# Add the target to iMSTK folder
#-----------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER iMSTK)

