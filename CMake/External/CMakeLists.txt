set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH} )
include(ExternalProject)

#-----------------------------------------------------------------------------
# Git protocol option
#-----------------------------------------------------------------------------
if(NOT GIT_EXECUTABLE)
  find_package(Git REQUIRED)
endif()

option(${PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." ON)
set(git_protocol "git")
if(NOT ${PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# CMake global args
#-----------------------------------------------------------------------------
list(APPEND CMAKE_CONFIG_ARGS
  -Wno-dev
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_INCLUDE_PATH:STRING=${${PROJECT_NAME}_CMAKE_INCLUDE_PATH}
  -DCMAKE_LIBRARY_PATH:STRING=${${PROJECT_NAME}_CMAKE_LIBRARY_PATH}
)

#-----------------------------------------------------------------------------
# CMake args if Apple
#-----------------------------------------------------------------------------
if(APPLE)
  list(APPEND CMAKE_CONFIG_OSX_ARGS
    -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT:PATH=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_MACOSX_RPATH:BOOL=ON
  )
endif()

#-----------------------------------------------------------------------------
# Solve dependencies
#-----------------------------------------------------------------------------
include(imstkSolveDependencies)
set(EXTERNAL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
ExternalProject_Include_Dependencies( ${PROJECT_NAME}
  DEPENDS_VAR ${PROJECT_NAME}_DEPENDENCIES
  SUPERBUILD_VAR ${PROJECT_NAME}_SUPERBUILD
  )

#-----------------------------------------------------------------------------
# Inner build of the main project
#-----------------------------------------------------------------------------
ExternalProject_Add( ${PROJECT_NAME}
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  DOWNLOAD_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/Innerbuild
  CMAKE_GENERATOR ${CMAKE_GENERATOR}
  CMAKE_ARGS
    -D${PROJECT_NAME}_SUPERBUILD:BOOL=OFF
#    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
#    -DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}
#    -DBUILD_EXAMPLES:BOOL=${BUILD_EXAMPLES}
#    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    ${CMAKE_CONFIG_ARGS}
    ${CMAKE_CONFIG_OSX_ARGS}
    ${${PROJECT_NAME}_EXTERNAL_PROJECTS_PATHS}
  DEPENDS ${${PROJECT_NAME}_DEPENDENCIES}
  )
