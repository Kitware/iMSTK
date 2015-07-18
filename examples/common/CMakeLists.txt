simmedtk_add_library(ExamplesCommon
  SOURCES
    wasdCameraController.cpp
    KeyPressSDKShutdown.cpp
    hapticController.cpp
    pzrMouseCameraController.cpp

  PUBLIC_HEADERS
    wasdCameraController.h
    KeyPressSDKShutdown.h
    hapticController.h
    pzrMouseCameraController.h
)

target_link_libraries(ExamplesCommon
  Core
  Rendering
  Event
  Simulators
  )
