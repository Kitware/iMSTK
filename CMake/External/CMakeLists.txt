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

set(VTK_PATCH_COMMAND)
if(NOT ${USE_SYSTEM_VTK} AND ${PROJECT_NAME}_VTK_REPO_SOURCE EQUAL "8.2")
  set(VTK_PATCH_COMMAND ${CMAKE_COMMAND}
    -DVTK_SOURCE_DIR=${VTK_SOURCE_DIR}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -P ${CMAKE_SOURCE_DIR}/CMake/External/Patches/vtk-8.2.cmake)
endif()

set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
if(WIN32)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -D${PROJECT_NAME}_ENABLE_AUDIO:BOOL=${${PROJECT_NAME}_AUDIO_ENABLED}
    )
endif()

#-----------------------------------------------------------------------------
# Inner build of the main project
#-----------------------------------------------------------------------------
ExternalProject_Add( ${PROJECT_NAME}
  ${${PROJECT_NAME}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/Innerbuild
  ${COMMON_CMAKE_EP_ARGS}
  CMAKE_ARGS
    -D${PROJECT_NAME}_SUPERBUILD:BOOL=OFF
    -D${PROJECT_NAME}_BUILD_EXAMPLES:BOOL=${${PROJECT_NAME}_BUILD_EXAMPLES}
    -D${PROJECT_NAME}_BUILD_TESTING:BOOL=${${PROJECT_NAME}_BUILD_TESTING}
    -D${PROJECT_NAME}_WRAP_CSHARP:BOOL=${${PROJECT_NAME}_WRAP_CSHARP}
    -D${PROJECT_NAME}_USE_OpenHaptics:BOOL=${${PROJECT_NAME}_USE_OpenHaptics}
    -D${PROJECT_NAME}_USE_MODEL_REDUCTION:BOOL=${${PROJECT_NAME}_USE_MODEL_REDUCTION}    
    -D${PROJECT_NAME}_USE_VRPN:BOOL=${${PROJECT_NAME}_USE_VRPN}
    -D${PROJECT_NAME}_USE_PHYSX:BOOL=${${PROJECT_NAME}_USE_PHYSX}
    -D${PROJECT_NAME}_COLOR_OUTPUT:BOOL=${${PROJECT_NAME}_COLOR_OUTPUT}
    -D${PROJECT_NAME}_USE_VTK_OSMESA:BOOL=${${PROJECT_NAME}_USE_VTK_OSMESA}
    -D${PROJECT_NAME}_COVERAGE:BOOL=${${PROJECT_NAME}_COVERAGE}
    # External Libraries
    -DEigen3_DIR:PATH=${Eigen3_DIR}    
    -DGTest_DIR:PATH=${GTest_DIR}
    -DVegaFEM_DIR:PATH=${VegaFEM_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    # I don't think this is needed by the inner build
    # -DPHYSX_CONFIGURATION:STRING=${${PROJECT_NAME}_PHYSX_CONFIGURATION}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
  DEPENDS ${${PROJECT_NAME}_DEPENDENCIES}
  PATCH_COMMAND 
    COMMAND ${VTK_PATCH_COMMAND}
    # Always build these non standard configurations
  )

ExternalProject_AlwaysConfigure(${PROJECT_NAME})

#-----------------------------------------------------------------------------
# Add the target to iMSTK folder
#-----------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER iMSTK)

