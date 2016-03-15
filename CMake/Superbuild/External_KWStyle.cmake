include(ExternalProject)

#-----------------------------------------------------------------------------
# Set KWStyle Git info
#-----------------------------------------------------------------------------
set(KWStyle_GIT_REPOSITORY "${git_protocol}://github.com/Kitware/KWStyle.git")
set(KWStyle_GIT_TAG "ef373a1ece313e9d096948e639bfb575f052f581")

#-----------------------------------------------------------------------------
# Set KWStyle directories
#-----------------------------------------------------------------------------
set(KWStyle_SOURCE_DIR ${CMAKE_BINARY_DIR}/Superbuild/KWStyle)
set(KWStyle_DIR ${CMAKE_BINARY_DIR}/Superbuild/KWStyle-build)
set(KWSTYLE_EXECUTABLE ${KWStyle_DIR}/KWStyle)

#-----------------------------------------------------------------------------
# Always build KWStyle on release mode
#-----------------------------------------------------------------------------
set(_build_configuration_arg -DCMAKE_BUILD_TYPE=Release)

#-----------------------------------------------------------------------------
# Download and build KWStyle
#-----------------------------------------------------------------------------
ExternalProject_add(KWStyle
  GIT_REPOSITORY ${KWStyle_GIT_REPOSITORY}
  GIT_TAG ${KWStyle_GIT_TAG}
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  DOWNLOAD_DIR ${KWStyle_SOURCE_DIR}
  SOURCE_DIR ${KWStyle_SOURCE_DIR}
  BINARY_DIR ${KWStyle_DIR}
  INSTALL_DIR ${KWStyle_DIR}
  LOG_DOWNLOAD 1
  LOG_UPDATE 0
  LOG_CONFIGURE 0
  LOG_BUILD 0
  LOG_TEST 0
  LOG_INSTALL 0
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_COMPILER_ARG1:STRING=${CMAKE_CXX_COMPILER_ARG1}
    -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
    -DCMAKE_C_COMPILER_ARG1:STRING=${CMAKE_C_COMPILER_ARG1}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    ${_build_configuration_arg}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DBUILD_TESTING:BOOL=OFF
  )
