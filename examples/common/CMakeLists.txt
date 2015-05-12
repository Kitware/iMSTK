simmedtk_add_library(ExamplesCommon
  SOURCES
    wasdCameraController.cpp
    KeyPressSDKShutdown.cpp
  PUBLIC_HEADERS
    wasdCameraController.h
    KeyPressSDKShutdown.h
)

target_link_libraries(ExamplesCommon
  smCore
  smRendering
  smEvent
  )
