#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_define_external_dirs( PhysX )

if(WIN32)
  set(PHYSX_TARGET_BUILD_PLATFORM windows)
elseif(APPLE)
  set(PHYSX_TARGET_BUILD_PLATFORM mac)
else()
  set(PHYSX_TARGET_BUILD_PLATFORM linux)
endif()

# PhysX has non standard build configurations
# A superbuild will fail if the configuration is not release or debug
# To prevent failure and support multiconfiguration builds,
# we explicitly build one or more configurations during our superbuild of PhysX
# (i.e. we don't let the superbuild pass down the configuration)

string(TOLOWER ${${PROJECT_NAME}_PHYSX_CONFIGURATION} PHYSX_CONFIGURATION)
set(PHYSX_BUILD_RELEASE "${CMAKE_COMMAND}"
                        --build . --config ${PHYSX_CONFIGURATION}
   )
set(PHYSX_BUILD_DEBUG "${CMAKE_COMMAND}"
                        --build . --config debug
   )
set(PHYSX_INSTALL_RELEASE "${CMAKE_COMMAND}"
                        --build . --target install --config ${PHYSX_CONFIGURATION}
   )
set(PHYSX_INSTALL_DEBUG "${CMAKE_COMMAND}"
                        --build . --target install --config debug
   )
if(NOT (MSVC OR XCode))
  # Build according to the specified configuration
  if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    unset(PHYSX_BUILD_RELEASE)
    unset(PHYSX_INSTALL_RELEASE)
  else()
    unset(PHYSX_BUILD_DEBUG)
    unset(PHYSX_INSTALL_DEBUG)
  endif()
  message(STATUS "Building a single PhysX configuration")
else()
  message(STATUS "Building dual PhysX configurations")
endif()

set(PHYSX_iMSTK_INSTALL "${CMAKE_COMMAND}"
  -DPhysX_INSTALL_DIR=${PhysX_BINARY_DIR}/install
  -DiMSTK_INSTALL_DIR=${CMAKE_INSTALL_PREFIX}
  -P ${CMAKE_SOURCE_DIR}/CMake/External/Patches/physx.cmake)

imstk_add_external_project( PhysX
  URL https://gitlab.kitware.com/iMSTK/PhysX/-/archive/f21d30499f9bcf1f165c208bc489e624c94e413e/archive.zip
  URL_MD5 e5296d372a34498b88226099a59417b1
  SOURCE_SUBDIR ./physx/compiler/public
  CMAKE_CACHE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${PHYSX_CONFIGURATION}
    -DCMAKE_INSTALL_PREFIX:PATH=${PhysX_BINARY_DIR}/install
    -DPXSHARED_INSTALL_PREFIX:PATH=${PhysX_BINARY_DIR}/install
    -DTARGET_BUILD_PLATFORM:STRING=${PHYSX_TARGET_BUILD_PLATFORM}
    -DPX_OUTPUT_ARCH:STRING=x86 # Either x86 or arm, only used for naming conventions (postfix)
    -DPHYSX_ROOT_DIR:PATH=${PhysX_SOURCE_DIR}/physx
    -DCMAKEMODULES_PATH:PATH=${PhysX_SOURCE_DIR}/externals/cmakemodules
    -DCMAKEMODULES_VERSION:STRING=1.27 # Must be set, but does not do anything...
    -DPXSHARED_PATH:PATH=${PhysX_SOURCE_DIR}/pxshared
    -DPX_OUTPUT_LIB_DIR:PATH=${PhysX_BINARY_DIR}
    -DPX_OUTPUT_BIN_DIR:PATH=${PhysX_BINARY_DIR}
    -DPX_BUILDSNIPPETS:BOOL=FALSE
    -DPX_BUILDPUBLICSAMPLES:BOOL=FALSE 
    -DPX_GENERATE_STATIC_LIBRARIES:BOOL=FALSE
    -DNV_USE_DEBUG_WINCRT:BOOL=TRUE 
    -DPX_FLOAT_POINT_PRECISE_MATH:BOOL=TRUE
    -DNV_FORCE_64BIT_SUFFIX:BOOL=FALSE
    -DNV_FORCE_32BIT_SUFFIX:BOOL=FALSE
    -DNV_USE_GAMEWORKS_OUTPUT_DIRS:BOOL=TRUE
    -DNV_APPEND_CONFIG_NAME:BOOL=TRUE
    #VERBOSE
  BUILD_COMMAND
    COMMAND ${PHYSX_BUILD_DEBUG}
    COMMAND ${PHYSX_BUILD_RELEASE}
  INSTALL_COMMAND
    COMMAND ${PHYSX_INSTALL_DEBUG}
    COMMAND ${PHYSX_INSTALL_RELEASE}
    COMMAND ${PHYSX_iMSTK_INSTALL}
)
