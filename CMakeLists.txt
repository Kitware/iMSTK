cmake_minimum_required(VERSION 3.2)
project(iMSTK VERSION 0.0.1 LANGUAGES C CXX)

#-----------------------------------------------------------------------------
# CTest/Dashboards
#-----------------------------------------------------------------------------
include(CTest)

#-----------------------------------------------------------------------------
# Update CMake module path & cmake dir
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utilities
    ${CMAKE_MODULE_PATH}
    )
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Project build directories
#-----------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_BIN_DIR)
  set(${PROJECT_NAME}_BIN_DIR "bin")
endif()
if(NOT DEFINED ${PROJECT_NAME}_LIB_DIR)
  set(${PROJECT_NAME}_LIB_DIR "lib")
endif()
if(NOT DEFINED ${PROJECT_NAME}_INCLUDE_DIR)
  set(${PROJECT_NAME}_INCLUDE_DIR "include")
endif()
if(NOT DEFINED ${PROJECT_NAME}_SHARE_DIR)
  set(${PROJECT_NAME}_SHARE_DIR "share")
endif()

#-----------------------------------------------------------------------------
# Project install directories
#-----------------------------------------------------------------------------
if(APPLE)
  set(${PROJECT_NAME}_INSTALL_ROOT "${${PROJECT_NAME}_MAIN_PROJECT_APPLICATION_NAME}.app/Contents") # Set to create Bundle
else()
  set(${PROJECT_NAME}_INSTALL_ROOT ".")
endif()
set(${PROJECT_NAME}_INSTALL_BIN_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/${${PROJECT_NAME}_BIN_DIR}/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
set(${PROJECT_NAME}_INSTALL_LIB_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/${${PROJECT_NAME}_LIB_DIR}/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/${${PROJECT_NAME}_INCLUDE_DIR}/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
set(${PROJECT_NAME}_INSTALL_SHARE_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/${${PROJECT_NAME}_SHARE_DIR}/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")

#-----------------------------------------------------------------------------
# C++11 Support
#-----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

#-----------------------------------------------------------------------------
# Define External dependencies
#-----------------------------------------------------------------------------
macro(imstk_define_dependency extProj)
  list(APPEND ${PROJECT_NAME}_DEPENDENCIES ${extProj})
  option(USE_SYSTEM_${extProj} "Exclude ${extProj} from superbuild and use an existing build." OFF)
  mark_as_advanced(USE_SYSTEM_${extProj})
endmacro()

option(${PROJECT_NAME}_USE_Uncrustify "Use Uncrustify as a code style beautifier." OFF)
if(${PROJECT_NAME}_USE_Uncrustify)
  imstk_define_dependency(Uncrustify)
endif()

if(WIN32)
  imstk_define_dependency(PThreads)
  imstk_define_dependency(Libusb) #for VRPN
  imstk_define_dependency(FTD2XX) #for LibNiFalcon
endif()

imstk_define_dependency(g3log)
imstk_define_dependency(Eigen)
imstk_define_dependency(SCCD)
imstk_define_dependency(VegaFEM)
imstk_define_dependency(VTK)
imstk_define_dependency(VRPN)
imstk_define_dependency(LibNiFalcon)

#-----------------------------------------------------------------------------
# SUPERBUILD
#-----------------------------------------------------------------------------
option(${PROJECT_NAME}_SUPERBUILD "Build ${PROJECT_NAME} and the projects it depends on." ON)

if(${PROJECT_NAME}_SUPERBUILD)
  # Call CMakeLists.txt in CMake/External which will solve the dependencies
  # and add the External projects, including this one: this top-level
  # CMakeLists.txt will be called back with SUPERBUILD=OFF, to execute
  # the rest of the code below (INNERBUILD), which explains the `return`
  add_subdirectory(CMake/External)
  return()
endif()

#-----------------------------------------------------------------------------
# INNERBUILD
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Find external dependencies
#-----------------------------------------------------------------------------
# Uncrustify
find_program(Uncrustify_EXECUTABLE Uncrustify)
include(SetupUncrustifyConfig)

# g3log
find_package( g3log REQUIRED )
include_directories( ${g3log_INCLUDE_DIR} )

# Eigen
find_package( Eigen 3.1.2 REQUIRED )
include_directories( ${Eigen_INCLUDE_DIR} )

# SCCD
find_package( SCCD REQUIRED )
include_directories( ${SCCD_INCLUDE_DIR} )

# VegaFEM
find_package( VegaFEM REQUIRED CONFIG )

# VTK
find_package( VTK REQUIRED CONFIG )
include( ${VTK_USE_FILE} )

# VRPN
find_package( VRPN REQUIRED )
include_directories( ${VRPN_INCLUDE_DIRS} )
add_definitions( -DVRPN_USE_LIBNIFALCON )
if(${PROJECT_NAME}_USE_OMNI)
  add_definitions( -DiMSTK_USE_OPENHAPTICS )
  add_definitions( -DVRPN_USE_PHANTOM_SERVER )
else()
  remove_definitions( -DiMSTK_USE_OPENHAPTICS )
  remove_definitions( -DVRPN_USE_PHANTOM_SERVER )
endif()

#--------------------------------------------------------------------------
# Add Source code subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Base/Core)
add_subdirectory(Base/Geometry)
add_subdirectory(Base/Devices)
add_subdirectory(Base/Rendering)
add_subdirectory(Base/Solvers)
add_subdirectory(Base/TimeIntegrators)
add_subdirectory(Base/ForceModel)
add_subdirectory(Base/SceneElements)
add_subdirectory(Base/Collision)
add_subdirectory(Base/Scene)
add_subdirectory(Base/SimulationManager)
add_subdirectory(Base/Constraint)
#--------------------------------------------------------------------------
# Export Targets
#--------------------------------------------------------------------------
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAMESPACE)
set(PROJECT_NAMESPACE "${PROJECT_NAMESPACE}::")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${Upstream_VERSION}
  COMPATIBILITY AnyNewerVersion
  )
export(EXPORT ${PROJECT_NAME}_TARGETS
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAMESPACE}
  )
configure_file(${PROJECT_NAME}Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  @ONLY
  )

install(EXPORT ${PROJECT_NAME}_TARGETS
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    ${PROJECT_NAMESPACE}
  DESTINATION
    ${${PROJECT_NAME}_INSTALL_SHARE_DIR}
  )
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION
    ${${PROJECT_NAME}_INSTALL_SHARE_DIR}
  COMPONENT
    Devel
  )

#--------------------------------------------------------------------------
# Add Examples subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Examples)
