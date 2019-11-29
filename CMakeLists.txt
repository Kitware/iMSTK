cmake_minimum_required(VERSION 3.9)
project(iMSTK VERSION 1.0.0 LANGUAGES C CXX)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

#-----------------------------------------------------------------------------
# CTest/Dashboards
#-----------------------------------------------------------------------------
include(CTest)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_DEBUG_POSTFIX "d")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install location" FORCE)
endif()
set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

#-----------------------------------------------------------------------------
# Project install directories
#-----------------------------------------------------------------------------
if(APPLE)
  set(${PROJECT_NAME}_INSTALL_ROOT "${CMAKE_INSTALL_PREFIX}/${${PROJECT_NAME}_MAIN_PROJECT_APPLICATION_NAME}.app/Contents") # Set to create Bundle
else()
  set(${PROJECT_NAME}_INSTALL_ROOT "${CMAKE_INSTALL_PREFIX}")
endif()
set(${PROJECT_NAME}_INSTALL_BIN_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/bin")
set(${PROJECT_NAME}_INSTALL_LIB_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/lib")
set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR "${${PROJECT_NAME}_INSTALL_ROOT}/include/imstk-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(${PROJECT_NAME}_INSTALL_SHARE_DIR "${${PROJECT_NAME}_INSTALL_ROOT}")
# Let's go ahead and make these directories
file(MAKE_DIRECTORY ${${PROJECT_NAME}_INSTALL_BIN_DIR})
file(MAKE_DIRECTORY ${${PROJECT_NAME}_INSTALL_LIB_DIR})
file(MAKE_DIRECTORY ${${PROJECT_NAME}_INSTALL_SHARE_DIR})

#-----------------------------------------------------------------------------
# Update CMake module path & cmake dir
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utilities
    ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_INSTALL_PREFIX}/lib/cmake # Vega and VTK
    )
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

#-----------------------------------------------------------------------------
# C++11 Support
#-----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
option(${PROJECT_NAME}_BUILD_EXAMPLES "Build iMSTK examples" ON)
option(${PROJECT_NAME}_BUILD_TESTING "Build iMSTK tests" ON)

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

  imstk_define_dependency(Assimp)
  imstk_define_dependency(Eigen)
  imstk_define_dependency(g3log)
  imstk_define_dependency(glm)
  imstk_define_dependency(imgui)
  imstk_define_dependency(LibNiFalcon)
  imstk_define_dependency(SCCD)
  imstk_define_dependency(tbb)
  imstk_define_dependency(VegaFEM)
  imstk_define_dependency(VTK)
  if(${PROJECT_NAME}_USE_OpenHaptics)
    imstk_define_dependency(OpenHaptics)
  endif()
  imstk_define_dependency(VRPN)

  if(WIN32)
    imstk_define_dependency(PThreads)
    imstk_define_dependency(Libusb) #for VRPN
    imstk_define_dependency(FTD2XX) #for LibNiFalcon
    imstk_define_dependency(SFML)
    set(${PROJECT_NAME}_AUDIO_ENABLED "Windows OS detected: Building iMSTK with AUDIO support." ON)
  else()
    message("Warning: Building iMSTK WITHOUT audio support!")
    set(${PROJECT_NAME}_AUDIO_ENABLED OFF)
  endif()

  if(${PROJECT_NAME}_USE_Vulkan)
    imstk_define_dependency(glfw)
    imstk_define_dependency(gli)
    imstk_define_dependency(Vulkan)
  endif()

  option(${PROJECT_NAME}_ENABLE_VR "Allow the usage of VR rendering." OFF)

  if(${PROJECT_NAME}_ENABLE_VR)
    imstk_define_dependency(openvr)
  endif()

  if(${PROJECT_NAME}_BUILD_TESTING)
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
#                               INNERBUILD
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Find external dependencies
#-----------------------------------------------------------------------------
include(imstkFind)


# Assimp
find_package( Assimp REQUIRED )
# Eigen
find_package( Eigen 3.1.2 REQUIRED )
if(WIN32)
  # FTD2XX
  find_package( FTD2XX REQUIRED )
endif()
# g3log
find_package( g3log REQUIRED )
# glm
find_package( glm REQUIRED)
# imgui
find_package( imgui REQUIRED )
# LibNiFalcon
find_package( LibNiFalcon REQUIRED)
# Libusb
find_package( Libusb REQUIRED)

# OpenVR
if(${PROJECT_NAME}_ENABLE_VR)
  add_definitions( -DiMSTK_ENABLE_VR )
  find_package( openvr REQUIRED )
else()
  remove_definitions( -DiMSTK_ENABLE_VR )
endif()

# SCCD
find_package( SCCD REQUIRED )

# SFML
if(APPLE OR LINUX)
  remove_definitions( -DiMSTK_AUDIO_ENABLED )
else()
  find_package( SFML REQUIRED )
  add_definitions( -DiMSTK_AUDIO_ENABLED )
endif()

# TBB
find_package(tbb REQUIRED)
# Define  __TBB_NO_IMPLICIT_LINKAGE so that MSVC will not always look for tbb_debug in debug mode
add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE)

# VegaFEM
find_package( VegaFEM REQUIRED CONFIG )

# VRPN
find_package( VRPN REQUIRED )
if(${PROJECT_NAME}_USE_OpenHaptics)
  find_package( OpenHapticsSDK REQUIRED )
  add_definitions( -DiMSTK_USE_OPENHAPTICS )
else()
  remove_definitions( -DiMSTK_USE_OPENHAPTICS )
endif()

# VTK
find_package(VTK CONFIG)
if (VTK_VERSION VERSION_LESS "8.90")
  # Modules are linked via `vtkCommonCore`
  # VTK_DEFINITIONS has autoinit information
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
else()
  # modules are linked via `VTK::CommonCore`
  # vtk_module_autoinit is needed
  find_package(VTK COMPONENTS
    CommonCore
    CommonDataModel
    FiltersGeneral
    FiltersSources
    ImagingCore
    InteractionStyle
    IOExport
    IOImport
    IOParallel
    IOParallelXML
    IOPLY
    RenderingCore
    RenderingAnnotation
    RenderingOpenGL2
  )
endif()

# Vulkan
set(Vulkan_Dependency)
if( ${PROJECT_NAME}_USE_Vulkan )
  find_package( VulkanSDK REQUIRED)
  add_definitions( -DiMSTK_USE_Vulkan )
  # glfw
  find_package( glfw REQUIRED)
  # gli
  find_package( gli REQUIRED)
  set(Vulkan_Dependency VulkanSDK)
else()
  remove_definitions( -DiMSTK_USE_Vulkan )
  unset(Vulkan_Dependency)
endif()


#--------------------------------------------------------------------------
# External Utility Packages
#--------------------------------------------------------------------------
include(imstkAddExecutable)
# Uncrustify
find_program(Uncrustify_EXECUTABLE uncrustify)
include(SetupUncrustifyConfig)
if(Uncrustify_EXECUTABLE)
  include(imstkAddUncrustifyCustomTarget)
else(Uncrustify_EXECUTABLE)
  message(WARNING "uncrustify not found! Cannot run code-style test.")
endif(Uncrustify_EXECUTABLE)

# Google Test
if(${PROJECT_NAME}_BUILD_TESTING)
  find_package( GoogleTest REQUIRED )
  include_directories(${GOOGLETEST_INCLUDE_DIRS})
  find_package( GoogleMock REQUIRED )
  include_directories(${GOOGLEMOCK_INCLUDE_DIRS})
endif()

#-----------------------------------------------------------------------------
# Download the external data needed for both testing and examples
#-----------------------------------------------------------------------------
if(${PROJECT_NAME}_BUILD_TESTING OR ${PROJECT_NAME}_BUILD_EXAMPLES)
  include(imstkExternalData)
  include(imstkExternalDataDownloadTest)
  imstk_add_data(${PROJECT_NAME} ${FILE_LIST})

  #-----------------------------------------------------------------------------
  # Add shaders
  #-----------------------------------------------------------------------------
  include(imstkCopyAndCompileShaders)
  CopyAndCompileShaders()
endif()

#--------------------------------------------------------------------------
# Add Source code subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Source/Core)
add_subdirectory(Source/Rendering/Materials)
add_subdirectory(Source/Geometry)#Circular Dependency
add_subdirectory(Source/DataStructures)#Circular Dependency
add_subdirectory(Source/Devices)
add_subdirectory(Source/Constraint)
add_subdirectory(Source/Rendering/GUIOverlay)
add_subdirectory(Source/Scene/SceneElements)
add_subdirectory(Source/Solvers)#Circular Dependency
add_subdirectory(Source/TimeIntegrators)#Circular Dependency
add_subdirectory(Source/DynamicalModels)#Circular Dependency
add_subdirectory(Source/Collision)
add_subdirectory(Source/Animation)
add_subdirectory(Source/Scene)
add_subdirectory(Source/Rendering)
add_subdirectory(Source/SimulationManager)
add_subdirectory(Source/apiUtilities)

#--------------------------------------------------------------------------
# Add Examples subdirectories
#--------------------------------------------------------------------------
if(${PROJECT_NAME}_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

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
