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

#-----------------------------------------------------------------------------
# Inner build of the main project
#-----------------------------------------------------------------------------
ExternalProject_Add( ${PROJECT_NAME}
  ${${PROJECT_NAME}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/Innerbuild
  CMAKE_ARGS
    -DBUILD_EXAMPLES:BOOL=${BUILD_EXAMPLES}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -D${PROJECT_NAME}_SUPERBUILD:BOOL=OFF
    -D${PROJECT_NAME}_USE_OMNI:BOOL=${${PROJECT_NAME}_USE_OMNI}
    -D${PROJECT_NAME}_USE_ODE:BOOL=${${PROJECT_NAME}_USE_ODE}
    -DODE_ROOT_DIR:PATH=${ODE_ROOT_DIR}
    -D${PROJECT_NAME}_USE_Vulkan:BOOL=${${PROJECT_NAME}_USE_Vulkan}
    -D${PROJECT_NAME}_ENABLE_AUDIO:BOOL=${${PROJECT_NAME}_ENABLE_AUDIO}
    -D${PROJECT_NAME}_ENABLE_VR:BOOL=${${PROJECT_NAME}_ENABLE_VR}
  DEPENDS ${${PROJECT_NAME}_DEPENDENCIES}
  )

	#-----------------------------------------------------------------------------
    # Add the target to Examples folder
    #-----------------------------------------------------------------------------
    SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES FOLDER iMSTK)
