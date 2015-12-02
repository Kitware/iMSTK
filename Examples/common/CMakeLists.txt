set(ExamplesCommon_sources)
set(ExamplesCommon_headers)
set(ExamplesCommon_link_libs)

list(APPEND ExamplesCommon_sources
  wasdCameraController.cpp
  KeyPressSDKShutdown.cpp
  hapticController.cpp
  pzrMouseCameraController.cpp
  ExampleCube.cpp
  TexturedSquare.cpp
)

list(APPEND ExamplesCommon_headers
  wasdCameraController.h
  KeyPressSDKShutdown.h
  hapticController.h
  pzrMouseCameraController.h
  ExampleCube.h
  TexturedSquare.h
)

list(APPEND ExamplesCommon_link_libs
  Core
  Rendering
  Event
  Simulators
  Geometry
  Mesh
)

simmedtk_add_library(ExamplesCommon
  SOURCES
    ${ExamplesCommon_sources}

  PUBLIC_HEADERS
    ${ExamplesCommon_headers}
)

target_link_libraries(ExamplesCommon
  ${ExamplesCommon_link_libs}
)