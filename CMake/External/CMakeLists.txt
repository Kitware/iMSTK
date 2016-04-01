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
# Output Directories
#-----------------------------------------------------------------------------
set(_sep_ "^^")

# Set directory variables
if(NOT BIN_DIR)
  set(BIN_DIR bin)
endif(NOT BIN_DIR)

if(NOT LIB_DIR)
  set(LIB_DIR lib)
endif(NOT LIB_DIR)

# Output directories - this is where built library and executable
# files will be placed after building but prior to install.  The
# necessary variables change between single and multi configuration
# build systems, so it is necessary to handle both cases on a
# conditional basis.
if(NOT CMAKE_CONFIGURATION_TYPES)
  # If we're not doing multi-configuration, just set the three main
  # variables to the correct values.
  if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKES_BINARY_DIR}/${LIB_DIR})
  endif()
  if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKES_BINARY_DIR}/${LIB_DIR})
  endif()
  if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKES_BINARY_DIR}/${BIN_DIR})
  endif()
  list(APPEND CMAKE_OUTPUT_DIRECTORIES
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    )
  set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${_sep_}${CMAKE_LIBRARY_PATH}")
  set(CMAKE_LIBRARY_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}${_sep_}${CMAKE_LIBRARY_PATH}")
else()

  # Multi-configuration is more difficult.  Not only do we need to
  # properly set the output directories, but we also need to
  # identify the "toplevel" directory for each configuration so
  # we can place files, documentation, etc. in the correct
  # relative positions.  Because files may be placed by CMake
  # without a build target to put them in their proper relative build
  # directory position using these paths, we must fully qualify them
  # without using CMAKE_CFG_INTDIR.
  #
  # We define directories that may not be quite "standard"
  # for a particular build tool - for example, native VS2010 projects use
  # another directory to denote CPU type being compiled for - but CMake only
  # supports multi-configuration setups having multiple configurations,
  # not multiple compilers.
  #
  # One additional wrinkle we must watch for here is the case where
  # a multi-configuration setup uses "." for its internal directory -
  # if that's the case, we need to just set the various config output
  # directories to the same value.
  foreach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    set(CFG_ROOT ${CMAKE_BINARY_DIR}/${CFG_TYPE})
    string(TOUPPER "${CFG_TYPE}" CFG_TYPE_UPPER)
    if(NOT "CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}")
      set("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}" ${CFG_ROOT}/${LIB_DIR})
    endif()
    if(NOT "CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}")
      set("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}" ${CFG_ROOT}/${LIB_DIR})
    endif()
    if(NOT "CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}")
      set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}" ${CFG_ROOT}/${BIN_DIR})
    endif()
    list(APPEND CMAKE_OUTPUT_DIRECTORIES
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}
      )
    set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}${_sep_}${CMAKE_LIBRARY_PATH}")
    set(CMAKE_LIBRARY_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}${_sep_}${CMAKE_LIBRARY_PATH}")
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Prepare include path to fill external projects include path
#-----------------------------------------------------------------------------
set(CMAKE_INCLUDE_PATH)

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
  -DCMAKE_LIBRARY_PATH:STRING=${CMAKE_LIBRARY_PATH}
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
    ${CMAKE_CONFIG_ARGS}
    ${CMAKE_CONFIG_OSX_ARGS}
    ${CMAKE_OUTPUT_DIRECTORIES}
    -DCMAKE_INCLUDE_PATH:STRING=${CMAKE_INCLUDE_PATH}
    ${${PROJECT_NAME}_EXTERNAL_PROJECTS_PATHS}
  DEPENDS ${${PROJECT_NAME}_DEPENDENCIES}
  )
