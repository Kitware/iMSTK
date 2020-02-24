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

string(TOLOWER ${${PROJECT_NAME}_PHYSX_CONFIGURATION} PHYSX_CONFIGURATION)

set(PHYSX_iMSTK_INSTALL "${CMAKE_COMMAND}"
  -DPhysX_INSTALL_DIR=${PhysX_BINARY_DIR}/install
  -DiMSTK_INSTALL_DIR=${CMAKE_INSTALL_PREFIX}
  -P ${CMAKE_SOURCE_DIR}/CMake/External/Patches/physx.cmake)

imstk_add_external_project( PhysX
  #URL https://github.com/NVIDIAGameWorks/PhysX/archive/4.1.zip
  #URL_MD5 79004323c3c39152a6c0491967e0a609
  #URL_MD5 32fdaddc4ad4e7e637faa86311eb1803
  #DOWNLOAD_DIR ${PhysX_TMP_DIR}
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/PhysX.git
  GIT_TAG f21d30499f9bcf1f165c208bc489e624c94e413e 
  GIT_SHALLOW TRUE
  GIT_CONFIG status.showUntrackedFiles=no
  SOURCE_SUBDIR ./physx/compiler/public
  CMAKE_CACHE_ARGS
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
    -DPX_GENERATE_STATIC_LIBRARIES:BOOL=TRUE
    -DNV_USE_DEBUG_WINCRT:BOOL=TRUE 
    -DPX_FLOAT_POINT_PRECISE_MATH:BOOL=TRUE
    -DNV_FORCE_64BIT_SUFFIX:BOOL=TRUE
    -DNV_FORCE_32BIT_SUFFIX:BOOL=FALSE
    -DNV_USE_GAMEWORKS_OUTPUT_DIRS:BOOL=TRUE 
    -DNV_APPEND_CONFIG_NAME:BOOL=TRUE
    #VERBOSE
  BUILD_COMMAND
    COMMAND "${CMAKE_COMMAND}" --build . --config ${PHYSX_CONFIGURATION}
  INSTALL_COMMAND
    COMMAND "${CMAKE_COMMAND}" --build . --target install --config ${PHYSX_CONFIGURATION}
    COMMAND ${PHYSX_iMSTK_INSTALL}
)

