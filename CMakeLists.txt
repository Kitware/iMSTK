cmake_minimum_required(VERSION 3.9)
project(iMSTK VERSION 5.0.0 LANGUAGES C CXX)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_DEBUG_POSTFIX "d")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-----------------------------------------------------------------------------
# Project install directories
#-----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install location" FORCE)
endif()
set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
# Let's go ahead and make these directories
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib) 

#-----------------------------------------------------------------------------
# Update CMake module path & CMake dir
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utilities
    ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_INSTALL_PREFIX}/lib/cmake # Vega and VTK
    )
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Add "mark_as_superbuild" CMake function to associate option with inner build
include(imstkSolveDependencies)

#-----------------------------------------------------------------------------
# C++11 Support
#-----------------------------------------------------------------------------
set(_msg "Setting C++ standard")
message(STATUS "${_msg}")
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "${_msg} - C++${CMAKE_CXX_STANDARD}")
if(NOT CMAKE_CXX_STANDARD MATCHES "^(11|14|17)$")
  message(FATAL_ERROR "CMAKE_CXX_STANDARD must be set to 11, 14 or 17")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

# Prevents a compiler error for Visual Studio 15.8
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  if(${MSVC_VERSION} GREATER_EQUAL 1915)
    add_definitions(-D_DISABLE_EXTENDED_ALIGNED_STORAGE)
  endif()
endif()

#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------

# SWIG 
option(${PROJECT_NAME}_WRAP_CSHARP "Build iMSTK-C# wrapper code and lib" OFF)
mark_as_superbuild(${PROJECT_NAME}_WRAP_CSHARP:BOOL)

# SWIG will generate managed array using pinning if this option is ON. Otherwise using P/Invoke default array marshalling.
option(${PROJECT_NAME}_SWIG_PINNED_ARRAY "Managed C# arrays using pinning" ON)
mark_as_advanced(${PROJECT_NAME}_SWIG_PINNED_ARRAY)
mark_as_superbuild(${PROJECT_NAME}_SWIG_PINNED_ARRAY:BOOL)

if (${PROJECT_NAME}_WRAP_CSHARP)
  find_package(SWIG REQUIRED)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  set(CMAKE_SWIG_FLAGS "")
endif()

# CUDA
option(${PROJECT_NAME}_ENABLE_CUDA_BACKEND "Enable iMSTK CUDA backend" OFF)
mark_as_superbuild(${PROJECT_NAME}_ENABLE_CUDA_BACKEND)
if (${PROJECT_NAME}_ENABLE_CUDA_BACKEND)
  include(CheckLanguage)
  check_language(CUDA)
  if (CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
	set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
	set(CUDA_PROPAGATE_HOST_FLAGS ON)
	if(NOT DEFINED CMAKE_CUDA_STANDARD)
	  set(CMAKE_CUDA_STANDARD 11)
	  set(CMAKE_CUDA_STANDARD_REQUIRED ON)
	endif()
  else()
	message(STATUS "WARNING: CUDA compiler NOT FOUND; CUDA backend not enabled!")
    set(${PROJECT_NAME}_ENABLE_CUDA_BACKEND OFF)
  endif()
endif ()

# Audio
if(WIN32)
  if(DEFINED ${PROJECT_NAME}_ENABLE_AUDIO)
    set(_enable_audio ${${PROJECT_NAME}_ENABLE_AUDIO})
    set(_reason "${PROJECT_NAME}_ENABLE_AUDIO is SET")
  else()
    set(_enable_audio ON)
    set(_reason "default initialization")
  endif()
else()
  set(_enable_audio OFF)
  set(_reason "not supported")
endif()
set(_msg "${PROJECT_NAME} Audio support")
message(STATUS "${_msg} - ${_enable_audio} (${_reason})")
set(${PROJECT_NAME}_AUDIO_ENABLED ${_enable_audio})

# General Options
option(${PROJECT_NAME}_BUILD_EXAMPLES "Build iMSTK examples" ON)
mark_as_superbuild(${PROJECT_NAME}_BUILD_EXAMPLES:BOOL)

option(${PROJECT_NAME}_BUILD_TESTING "Build iMSTK tests" ON)
mark_as_superbuild(${PROJECT_NAME}_BUILD_TESTING:BOOL)

set(BUILD_TESTING OFF)
if (${PROJECT_NAME}_BUILD_TESTING)
  set(BUILD_TESTING ON)
endif ()

#Build Benchmark
option(${PROJECT_NAME}_BUILD_BENCHMARK "Build iMSTK benchmarks" OFF)
mark_as_superbuild(${PROJECT_NAME}_BUILD_BENCHMARK:BOOL)


if (UNIX)
  option(${PROJECT_NAME}_USE_MODEL_REDUCTION "Build with model reduction, requires a VegaFEM built with Intel MKL and arpack" OFF)
  mark_as_superbuild(${PROJECT_NAME}_USE_MODEL_REDUCTION:BOOL)

  option(${PROJECT_NAME}_COLOR_OUTPUT "For coloring output in gcc" OFF)
  mark_as_superbuild(${PROJECT_NAME}_COLOR_OUTPUT:BOOL)

  option(${PROJECT_NAME}_COVERAGE "Build for calculating coverage" OFF)
  mark_as_superbuild(${PROJECT_NAME}_COVERAGE:BOOL)

  option(${PROJECT_NAME}_USE_VTK_OSMESA "Build with VTK OSMesa rendering" OFF)
  mark_as_superbuild(${PROJECT_NAME}_USE_VTK_OSMESA:BOOL)

  if (${PROJECT_NAME}_COVERAGE AND NOT ${CMAKE_BUILD_TYPE} MATCHES [Dd][Ee][Bb][Uu][Gg])
    message(WARNING "Coverage requires a debug build forcing debug")
    set(CMAKE_BUILD_TYPE "DEBUG")
  endif()  
endif()

option(${PROJECT_NAME}_USE_VRPN "Build with VRPN support" OFF)
mark_as_superbuild(${PROJECT_NAME}_USE_VRPN:BOOL)

option(${PROJECT_NAME}_USE_PHYSX "Build with Physx support" OFF)
mark_as_superbuild(iMSTK_USE_PHYSX:BOOL)
if(iMSTK_USE_PHYSX)
  set(PHYSX_CONFIGURATION "RELEASE" CACHE STRING "PhysX release library type")
  mark_as_superbuild(PHYSX_CONFIGURATION:STRING)

  # Set the possible values for cmake-gui
  set_property(CACHE PHYSX_CONFIGURATION PROPERTY STRINGS
    "RELEASE"
    "CHECKED"
    "PROFILE"
    )
endif()

option(iMSTK_USE_OpenHaptics "Use OpenHaptic Support." OFF)
mark_as_superbuild(iMSTK_USE_OpenHaptics:BOOL)
if(NOT WIN32 AND iMSTK_USE_OpenHaptics)
  message(FATAL_ERROR "Setting iMSTK_USE_OpenHaptics to ON is only supported on Windows")
endif()

option(${PROJECT_NAME}_BUILD_BENCHMARK "Build with benchmark support" ON)
mark_as_superbuild(${PROJECT_NAME}_BUILD_BENCHMARK)



#-----------------------------------------------------------------------------
# CTest/Dashboards
#-----------------------------------------------------------------------------
if (${PROJECT_NAME}_BUILD_TESTING)
  include(CTest)
  set_property(CACHE BUILD_TESTING PROPERTY TYPE INTERNAL)
endif ()

#-----------------------------------------------------------------------------
# SUPERBUILD
#-----------------------------------------------------------------------------
option(${PROJECT_NAME}_SUPERBUILD "Build ${PROJECT_NAME} and the projects it depends on." ON)

if(${PROJECT_NAME}_SUPERBUILD)

  #-----------------------------------------------------------------------------
  # Define External dependencies
  #-----------------------------------------------------------------------------
  macro(imstk_define_dependency extProj)
    list(APPEND ${PROJECT_NAME}_DEPENDENCIES ${extProj})
    option(USE_SYSTEM_${extProj} "Exclude ${extProj} from superbuild and use an existing build." OFF)
    mark_as_advanced(USE_SYSTEM_${extProj})
  endmacro()

  option(${PROJECT_NAME}_USE_Uncrustify "Use Uncrustify as a code style beautifier." ON)
  if(${PROJECT_NAME}_USE_Uncrustify)
    find_package (Python COMPONENTS Interpreter)
    if(Python_Interpreter_FOUND)
      imstk_define_dependency(Uncrustify)
    else()
      message(WARN "Uncrustify requires a python interpreter, please install python and rerun configure")
    endif()
  endif()
  
  imstk_define_dependency(Assimp)
  imstk_define_dependency(Eigen3)
  imstk_define_dependency(g3log)
  imstk_define_dependency(LibNiFalcon)
  imstk_define_dependency(OpenVR)
  if (${PROJECT_NAME}_USE_PHYSX)
    imstk_define_dependency(PhysX)
  endif()
  imstk_define_dependency(TBB)
  imstk_define_dependency(VegaFEM)
  imstk_define_dependency(VTK)
  if(${PROJECT_NAME}_USE_OpenHaptics)
    imstk_define_dependency(OpenHaptics)
  endif()
  if (${PROJECT_NAME}_USE_VRPN)
    imstk_define_dependency(VRPN)
  endif()

  if(WIN32)
    imstk_define_dependency(Libusb) #for VRPN
    imstk_define_dependency(FTD2XX) #for LibNiFalcon
    if(${PROJECT_NAME}_AUDIO_ENABLED)
      imstk_define_dependency(SFML)
    endif()
  endif()

  if(${PROJECT_NAME}_BUILD_TESTING)
    imstk_define_dependency(GTest)

    #-----------------------------------------------------------------------------
    # Allow CTest to cover Innerbuild
    #-----------------------------------------------------------------------------
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/CMake/Utilities/imstkCTestAddInnerbuild.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/imstkCTestAddInnerbuild.cmake"
      @ONLY
    )
    set_directory_properties(PROPERTIES TEST_INCLUDE_FILE
      "${CMAKE_CURRENT_BINARY_DIR}/imstkCTestAddInnerbuild.cmake"
    )
  endif()

  if (${PROJECT_NAME}_BUILD_TESTING OR ${PROJECT_NAME}_BUILD_EXAMPLES)
    imstk_define_dependency(iMSTKData)
  endif()


  # Build with Google Benchmark for benchmarking
  if (${PROJECT_NAME}_BUILD_BENCHMARK)
    imstk_define_dependency(benchmark)
  endif()


  #-----------------------------------------------------------------------------
  # Solve project dependencies
  #-----------------------------------------------------------------------------
  # Call CMakeLists.txt in CMake/External which will solve the dependencies
  # and add the External projects, including this one: this top-level
  # CMakeLists.txt will be called back with SUPERBUILD=OFF, to execute
  # the rest of the code below (INNERBUILD), which explains the `return`
  add_subdirectory(CMake/External)

  return()

endif()

#-----------------------------------------------------------------------------
#                               INNERBUILD
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Find external dependencies
#-----------------------------------------------------------------------------
include(imstkFind)
# Set where to look for packages (If not using system)
set(PACKAGE_PREFIX_DIR ${CMAKE_INSTALL_PREFIX})

# Assimp
find_package( Assimp REQUIRED )
# Eigen
find_package( Eigen3 3.1.2 REQUIRED )
if(WIN32)
  # FTD2XX
  find_package( FTD2XX REQUIRED )
endif()
# g3log
find_package( g3log REQUIRED )
# LibNiFalcon
find_package( LibNiFalcon REQUIRED)
# Libusb
find_package( Libusb REQUIRED)
# OpenVR
find_package( OpenVR REQUIRED )

# PhysX
if (iMSTK_USE_PHYSX)
  add_compile_definitions(IMSTK_USE_PHYSX)
  # Variable PHYSX_CONFIGURATION used in FindPhysX.cmake is set above where
  # iMSTK_USE_PHYSX CMake option is defined.
  find_package(PhysX REQUIRED)
endif()

# SFML
if(NOT iMSTK_AUDIO_ENABLED)
  remove_definitions( -DiMSTK_AUDIO_ENABLED )
else()
  find_package( SFML REQUIRED )
  add_definitions( -DiMSTK_AUDIO_ENABLED )
endif()

# TBB
find_package(TBB REQUIRED)

# VegaFEM
find_package( VegaFEM REQUIRED CONFIG )

# VRPN
if (${PROJECT_NAME}_USE_VRPN)
  find_package( VRPN REQUIRED )
  add_definitions( -DiMSTK_USE_VRPN )
else()
  remove_definitions( -DiMSTK_USE_VRPN )
endif()

# OpenHaptics
if(${PROJECT_NAME}_USE_OpenHaptics)
  find_package( OpenHapticsSDK REQUIRED )
  add_definitions( -DiMSTK_USE_OPENHAPTICS )
else()
  remove_definitions( -DiMSTK_USE_OPENHAPTICS )
endif()

# VTK
find_package(VTK REQUIRED CONFIG)
# modules are linked via `VTK::CommonCore`
# vtk_module_autoinit is needed

find_package(vtkRenderingOpenVR QUIET)
find_package(vtkRenderingExternal QUIET)

list(APPEND iMSTK_VTK_DEPENDENCIES
  ChartsCore
  CommonCore
  CommonDataModel
  FiltersCore
  FiltersExtraction
  FiltersGeneral
  FiltersModeling
  FiltersSources
  IOExport
  IOImport
  IOPLY
  IOParallel
  IOParallelXML
  ImagingCore
  ImagingGeneral
  ImagingStencil
  InteractionStyle
  RenderingAnnotation
  RenderingContext2D
  RenderingContextOpenGL2
  RenderingCore
  RenderingExternal
  RenderingOpenGL2
  RenderingOpenVR
  RenderingVolume
  RenderingVolumeOpenGL2
  ViewsContext2D)
find_package(VTK COMPONENTS ${iMSTK_VTK_DEPENDENCIES} REQUIRED)
if(${PROJECT_NAME}_USE_VTK_OSMESA)
  add_definitions(-DiMSTK_USE_VTK_OSMESA)
endif()

#--------------------------------------------------------------------------
# External Utility Packages
#--------------------------------------------------------------------------
include(imstkAddExecutable)
# Uncrustify
find_program(Uncrustify_EXECUTABLE uncrustify)
# include(SetupUncrustifyConfig)
if(Uncrustify_EXECUTABLE)
  include(imstkAddUncrustifyCustomTarget)
else(Uncrustify_EXECUTABLE)
  message(WARNING "uncrustify not found! Cannot run code-style test.")
endif(Uncrustify_EXECUTABLE)


#-----------------------------------------------------------------------------
# Data
#----------------------------------------------------------------------
if (${PROJECT_NAME}_BUILD_TESTING OR ${PROJECT_NAME}_BUILD_EXAMPLES)
  add_definitions( -DiMSTK_DATA_ROOT=\"${CMAKE_INSTALL_PREFIX}/data/\")
  
  # Note if the target name or data name changes this has to be changed as well
  add_custom_target(CopyDataFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/../External/iMSTKData/src/Data
    ${CMAKE_INSTALL_PREFIX}/data
    )
endif()



#-----------------------------------------------------------------------------
# Testing
#----------------------------------------------------------------------
if(${PROJECT_NAME}_BUILD_TESTING)
  find_package( GTest CONFIG REQUIRED )
endif()

#------------------------------------------------------------------
# Benchmarking
#------------------------------------------------------------------
if (${PROJECT_NAME}_BUILD_BENCHMARK)
  find_package( benchmark REQUIRED )
endif()


#-----------------------------------------------------------------------------
# Code Coverage
#----------------------------------------------------------------------
if(${PROJECT_NAME}_COVERAGE)
  # Note HS Had to add ALL in setup_target_for_coverage to force the code coverage pass 
  set(CODE_COVERAGE_VERBOSE ON)
  include(CodeCoverage)
  append_coverage_compiler_flags()
  setup_target_for_coverage_lcov(
    NAME Coverage
    EXECUTABLE ctest -E "(CodeFormatTest|RenderTest.*)"
    EXCLUDE "${CMAKE_BINARY_DIR}/../install/include/*" "/usr/*" "*/Testing/*"
  )
endif()

# Folder name to put our headers/cmake config files under
set(${PROJECT_NAME}_INSTALL_FOLDER ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR})

#--------------------------------------------------------------------------
# Add Source code subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Source/Common)
add_subdirectory(Source/Materials)
add_subdirectory(Source/Geometry)
add_subdirectory(Source/MeshIO)
add_subdirectory(Source/GeometryMappers)
add_subdirectory(Source/DataStructures)
add_subdirectory(Source/Constraint)
add_subdirectory(Source/Devices)
add_subdirectory(Source/RenderingCore)
add_subdirectory(Source/RenderingVTK)
add_subdirectory(Source/Solvers)
add_subdirectory(Source/DynamicalModels)
add_subdirectory(Source/SceneEntities)
add_subdirectory(Source/Animation)
add_subdirectory(Source/Controllers)
add_subdirectory(Source/CollisionDetection)
add_subdirectory(Source/CollisionHandling)
add_subdirectory(Source/Scene)
add_subdirectory(Source/SimulationManager)
add_subdirectory(Source/ViewerCore)
add_subdirectory(Source/ViewerVTK)
add_subdirectory(Source/Testing)
add_subdirectory(Source/Filtering)
add_subdirectory(Source/FilteringCore)

if (${PROJECT_NAME}_WRAP_CSHARP)
  add_subdirectory(Source/Wrappers)
endif()

#--------------------------------------------------------------------------
# Add Examples subdirectories
#--------------------------------------------------------------------------
if(${PROJECT_NAME}_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

#--------------------------------------------------------------------------
# Testing
#--------------------------------------------------------------------------
if (${PROJECT_NAME}_BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#--------------------------------------------------------------------------
# Add setup script for *nix systems
#--------------------------------------------------------------------------
if(NOT WIN32)
  # Create setup shell script to create an environment for running examples
  set(LIBRARY_PATH_VAR "LD_LIBRARY_PATH")
  if( APPLE )
    set(LIBRARY_PATH_VAR "DYLD_FALLBACK_LIBRARY_PATH")
  endif()
  configure_file(
    ${CMAKE_SOURCE_DIR}/CMake/setup_iMSTK.sh.in
    ${CMAKE_INSTALL_PREFIX}/setup_iMSTK.sh
    @ONLY)
endif()

#--------------------------------------------------------------------------
# Install Find Modules
#--------------------------------------------------------------------------
set(${PROJECT_NAME}_INSTALL_CONFIG_DIR "lib/cmake/${${PROJECT_NAME}_INSTALL_FOLDER}")
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${${PROJECT_NAME}_INSTALL_CONFIG_DIR}/modules)
file(GLOB modules ${CMAKE_SOURCE_DIR}/CMake/Find*.cmake)
foreach(module ${modules})
  install(FILES ${module} DESTINATION "${${PROJECT_NAME}_INSTALL_CONFIG_DIR}/modules")
endforeach()
install(FILES ${CMAKE_SOURCE_DIR}/CMake/Utilities/imstkFind.cmake DESTINATION "${${PROJECT_NAME}_INSTALL_CONFIG_DIR}/modules")

#--------------------------------------------------------------------------
# Innerbuild dummy test
#--------------------------------------------------------------------------
add_test(
  NAME imstkDummyTest
  COMMAND ${CMAKE_COMMAND} -E echo "Success"
)

#--------------------------------------------------------------------------
# Export Targets
#--------------------------------------------------------------------------
# This variable controls the prefix used to generate the following files:
#  ${PROJECT_NAME}ConfigVersion.cmake
#  ${PROJECT_NAME}Config.cmake
#  ${PROJECT_NAME}Targets.cmake
# and it also used to initialize ${PROJECT_NAME}_INSTALL_CONFIG_DIR value.
set(export_config_name ${PROJECT_NAME})
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAMESPACE)
set(PROJECT_NAMESPACE "${PROJECT_NAMESPACE}::")
#------------------------------------------------------------------------------
# Configure ${PROJECT_NAME}ConfigVersion.cmake common to build and install tree
include(CMakePackageConfigHelpers)
set(config_version_file ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
write_basic_package_version_file(
  ${config_version_file}
  VERSION "${${PROJECT_NAME}_VERSION}"
  COMPATIBILITY AnyNewerVersion
  )
#------------------------------------------------------------------------------
# Export '${PROJECT_NAME}Targets.cmake' for a build tree
export(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAMESPACE}
  )
# Configure '${PROJECT_NAME}Config.cmake' for a build tree
set(CONFIG_CODE "####### Expanded from \@CONFIG_CODE\@ #######\n")

set(CONFIG_CODE "${CONFIG_CODE}set(Assimp_ROOT_DIR \"${Assimp_ROOT_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(Assimp_LIB_DIR \"${Assimp_LIB_DIR}\")\n")

set(CONFIG_CODE "${CONFIG_CODE}set(Eigen3_DIR \"${Eigen3_DIR}\")\n")

set(CONFIG_CODE "${CONFIG_CODE}set(g3log_ROOT_DIR \"${g3log_ROOT_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(g3log_LIB_DIR \"${g3log_LIB_DIR}\")\n")

set(CONFIG_CODE "${CONFIG_CODE}set(GTest_DIR \"${GTest_DIR}\")\n")

if(WIN32)
  set(CONFIG_CODE "${CONFIG_CODE}# Required by FindFTD2XX CMake module used in and provided by the iMSTK project\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(FTD2XX_ROOT_DIR \"${FTD2XX_ROOT_DIR}\")\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(FTD2XX_LIB_DIR \"${FTD2XX_LIB_DIR}\")\n")
  set(CONFIG_CODE "${CONFIG_CODE}# Required by Findlibftd2xx CMake module provided and used by the LibNiFalcon project\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(LIBFTD2XX_INCLUDE_DIR \"${LIBFTD2XX_INCLUDE_DIR}\")\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(LIBFTD2XX_LIBRARY \"${LIBFTD2XX_LIBRARY}\")\n")
endif()

set(CONFIG_CODE "${CONFIG_CODE}set(LibNiFalcon_ROOT_DIR \"${LibNiFalcon_ROOT_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(LibNiFalcon_LIB_DIR \"${LibNiFalcon_LIB_DIR}\")\n")

if(WIN32)
  set(CONFIG_CODE "${CONFIG_CODE}set(Libusb_ROOT_DIR \"${Libusb_ROOT_DIR}\")\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(Libusb_LIB_DIR \"${Libusb_LIB_DIR}\")\n")
else()
  set(CONFIG_CODE "${CONFIG_CODE}set(Libusb_INCLUDE_DIR \"${Libusb_INCLUDE_DIR}\")\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(Libusb_LIBRARY_libusb-1.0-RELEASE \"${Libusb_LIBRARY_libusb-1.0-RELEASE}\")\n")
  set(CONFIG_CODE "${CONFIG_CODE}set(Libusb_LIBRARY_libusb-1.0-DEBUG \"${Libusb_LIBRARY_libusb-1.0-DEBUG}\")\n")
endif()

set(CONFIG_CODE "${CONFIG_CODE}# Add variables to CACHE to workaround issue related to use of\n")
set(CONFIG_CODE "${CONFIG_CODE}# mark_as_advanced command in FindOpenVR and described in CMP0102\n")
set(CONFIG_CODE "${CONFIG_CODE}# See https://cmake.org/cmake/help/latest/policy/CMP0102.html\n")
set(CONFIG_CODE "${CONFIG_CODE}set(OpenVR_INCLUDE_DIR \"${OpenVR_INCLUDE_DIR}\" CACHE PATH \"\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(OpenVR_LIBRARY \"${OpenVR_LIBRARY}\" CACHE FILEPATH \"\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(OpenVR_ROOT_DIR \"${OpenVR_ROOT_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(OpenVR_LIB_DIR \"${OpenVR_LIB_DIR}\")\n")

set(CONFIG_CODE "${CONFIG_CODE}set(TBB_DIR \"${TBB_DIR}\")\n")

set(CONFIG_CODE "${CONFIG_CODE}set(VegaFEM_DIR \"${VegaFEM_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(VTK_DIR \"${VTK_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(vtkRenderingOpenVR_DIR \"${vtkRenderingOpenVR_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(vtkRenderingExternal_DIR \"${vtkRenderingExternal_DIR}\")\n")

set(CONFIG_CODE "${CONFIG_CODE}##################################################")
set(build_config ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
configure_package_config_file(
  CMake/${PROJECT_NAME}Config.cmake.in
  ${build_config}
  INSTALL_DESTINATION "${PROJECT_BINARY_DIR}"
  )

#------------------------------------------------------------------------------
# Export '${PROJECT_NAME}Targets.cmake' for an install tree
install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAMESPACE}
  DESTINATION ${${PROJECT_NAME}_INSTALL_CONFIG_DIR}
  )
# Configure '${PROJECT_NAME}Config.cmake' for an install tree
set(CONFIG_CODE "####### Expanded from \@CONFIG_CODE\@ #######\n")
# Note: iMSTK specific variables likes <proj>_ROOT_DIR and <proj>_LIB_DIR
# are not set because the location of the corresponding project is automatically
# inferred by Find<proj> CMake module provided by iSMTK.
set(CONFIG_CODE "${CONFIG_CODE}set(Eigen3_DIR \"${Eigen3_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(GTest_DIR \"${GTest_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(TBB_DIR \"${TBB_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(VegaFEM_DIR \"${VegaFEM_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(VTK_DIR \"${VTK_DIR}\")\n")
set(CONFIG_CODE "${CONFIG_CODE}# Add variables to CACHE to workaround issue related to use of\n")
set(CONFIG_CODE "${CONFIG_CODE}# mark_as_advanced command in FindOpenVR and described in CMP0102\n")
set(CONFIG_CODE "${CONFIG_CODE}# See https://cmake.org/cmake/help/latest/policy/CMP0102.html\n")
set(CONFIG_CODE "${CONFIG_CODE}set(OpenVR_INCLUDE_DIR \"${OpenVR_INCLUDE_DIR}\" CACHE PATH \"\")\n")
set(CONFIG_CODE "${CONFIG_CODE}set(OpenVR_LIBRARY \"${OpenVR_LIBRARY}\" CACHE FILEPATH \"\")\n")
set(CONFIG_CODE "${CONFIG_CODE}##################################################")

set(install_config ${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake)
configure_package_config_file(
  CMake/${PROJECT_NAME}Config.cmake.in 
  ${install_config}
  INSTALL_DESTINATION ${${PROJECT_NAME}_INSTALL_CONFIG_DIR}
  )

# Install config files
install(
  FILES ${config_version_file} ${install_config}
  DESTINATION "${${PROJECT_NAME}_INSTALL_CONFIG_DIR}"
  )
