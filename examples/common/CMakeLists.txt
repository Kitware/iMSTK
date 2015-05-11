simmedtk_add_library(ExamplesCommon
  SOURCES
    wasdCameraController.cpp
  PUBLIC_HEADERS
    wasdCameraController.h
)

target_link_libraries(ExamplesCommon
  smCore
  smRendering
  smEvent
  )
