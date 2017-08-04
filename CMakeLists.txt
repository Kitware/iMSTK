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

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

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
    imstk_define_dependency(Uncrustify)
  endif()

  option(${PROJECT_NAME}_USE_Vulkan "Use the custom Vulkan renderer." OFF)

  if(${PROJECT_NAME}_USE_Vulkan)
    imstk_define_dependency(glfw)
    imstk_define_dependency(glm)
    imstk_define_dependency(gli)
  endif()

  if(WIN32)
    imstk_define_dependency(PThreads)
    imstk_define_dependency(Libusb) #for VRPN
    imstk_define_dependency(FTD2XX) #for LibNiFalcon
  endif()

  imstk_define_dependency(Assimp)
  imstk_define_dependency(g3log)
  imstk_define_dependency(Eigen)
  imstk_define_dependency(SCCD)
  imstk_define_dependency(VegaFEM)
  imstk_define_dependency(VTK)
  imstk_define_dependency(VRPN)
  imstk_define_dependency(LibNiFalcon)
  imstk_define_dependency(SFML)

  option(${PROJECT_NAME}_USE_ODE "Build iMSTK with ODE support." OFF)
  if(${PROJECT_NAME}_USE_ODE)
    imstk_define_dependency(ODE)
  endif()

  #----------------------------------------------------
  # Setting up ODE from preinstalled directories.
  #----------------------------------------------------
  #include(CMake/External/External_ODE.cmake)

  if(BUILD_TESTING)
    imstk_define_dependency(GoogleTest)

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
# INNERBUILD
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Find external dependencies
#-----------------------------------------------------------------------------
# Uncrustify
find_program(Uncrustify_EXECUTABLE uncrustify)
include(SetupUncrustifyConfig)
if(Uncrustify_EXECUTABLE)
  # Add target to run uncrustify
  add_custom_target(uncrustifyRun
    COMMAND ${Uncrustify_EXECUTABLE}
      -c ${CMAKE_CURRENT_LIST_DIR}/Utilities/Uncrustify/iMSTKUncrustify.cfg
      -F ${CMAKE_CURRENT_BINARY_DIR}/Uncrustify.list
      --no-backup
    COMMENT "Run uncrustify - overwrites source files"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

  if(BUILD_TESTING)
    # Add test to check style using uncrustify
    add_test(NAME uncrustifyCheck
      COMMAND ${Uncrustify_EXECUTABLE}
      -c ${CMAKE_CURRENT_LIST_DIR}/Utilities/Uncrustify/iMSTKUncrustify.cfg
      -F ${CMAKE_CURRENT_BINARY_DIR}/Uncrustify.list
      --check
      COMMENT "Run uncrustify in check mode"
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
      )
  endif(BUILD_TESTING)

  # Write list of files on which to run uncrustify.
  # Explicitly write a zero byte file when no source files are specified.
  # Otherwise, configure_file() creates a file with a single blank line, and
  # uncrustify interprets the blank line as a file name.
  file(GLOB_RECURSE UNCRUSTIFY_SOURCE_FILES *.h *.cpp)
  set(_uncrustify_list_in ${CMAKE_CURRENT_LIST_DIR}/Utilities/Uncrustify/uncrustify.list.in)
  set(_uncrustify_list_out ${CMAKE_CURRENT_BINARY_DIR}/uncrustify.list)
  if(UNCRUSTIFY_SOURCE_FILES)
    list(SORT UNCRUSTIFY_SOURCE_FILES)
    string(REPLACE ";" "\n" UNCRUSTIFY_SOURCE_FILES "${UNCRUSTIFY_SOURCE_FILES}")
    configure_file("${_uncrustify_list_in}" "${_uncrustify_list_out}" @ONLY)
  else()
    file(WRITE "${_uncrustify_list_out}")
  endif()
else(Uncrustify_EXECUTABLE)
  message(WARNING "uncrustify not found! Cannot run code-style test.")
endif(Uncrustify_EXECUTABLE)

# Assimp
find_package( Assimp REQUIRED )
include_directories( ${Assimp_INCLUDE_DIRS} )

if( ${PROJECT_NAME}_USE_Vulkan )
  # glfw
  find_package(glfw)
  include_directories( ${glfw_INCLUDE_DIR} )

  # glm
  find_package(glm)
  include_directories( ${glm_INCLUDE_DIR} )

  # gli
  find_package(gli)
  include_directories( ${gli_INCLUDE_DIR} )
endif()

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

# ODE
if(${PROJECT_NAME}_USE_ODE)
    find_package(ODE)
    # Assuming that ODE was compiled with Double precision.
    add_definitions( -DdDOUBLE ) # Needed by ODE library.
    add_definitions( -DiMSTK_USE_ODE )
else()
    remove_definitions( -DdDouble )
    remove_definitions( -DiMSTK_USE_ODE )
endif()

# Vulkan
if(${PROJECT_NAME}_USE_Vulkan)
  find_package( VulkanSDK )
  include_directories( ${VulkanSDK_INCLUDE_DIR} )
  add_definitions( -DiMSTK_USE_Vulkan )
else()
  remove_definitions( -DiMSTK_USE_Vulkan )
endif()

# SFML
find_package( SFML REQUIRED )

# Google Test
if(BUILD_TESTING)
  find_package( GoogleTest REQUIRED )
  include_directories(${GoogleTest_INCLUDE_DIRS})
  find_package( GoogleMock REQUIRED )
  include_directories(${GoogleMock_INCLUDE_DIRS})
endif()

# External data
if(BUILD_TESTING OR BUILD_EXAMPLES )
  include(imstkExternalData)
endif()

#--------------------------------------------------------------------------
# Add Source code subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Source/Core)
add_subdirectory(Source/Geometry)
add_subdirectory(Source/Devices)
add_subdirectory(Source/Rendering)
add_subdirectory(Source/Solvers)
add_subdirectory(Source/DynamicalModels)
add_subdirectory(Source/TimeIntegrators)
add_subdirectory(Source/SceneElements)
add_subdirectory(Source/Collision)
add_subdirectory(Source/Scene)
add_subdirectory(Source/SimulationManager)
add_subdirectory(Source/Constraint)
add_subdirectory(Source/Materials)

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

#--------------------------------------------------------------------------
# Innerbuild dummy test
#--------------------------------------------------------------------------
add_test(
  NAME imstkDummyTest
  COMMAND ${CMAKE_COMMAND} -E echo "Success"
)
