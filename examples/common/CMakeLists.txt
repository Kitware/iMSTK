simmedtk_add_library(ExamplesCommon
  SOURCES
    wasdCameraController.cpp
    KeyPressSDKShutdown.cpp
    hapticController.cpp
	
  PUBLIC_HEADERS
    wasdCameraController.h
    KeyPressSDKShutdown.h
	hapticController.h
)

target_link_libraries(ExamplesCommon
  smCore
  smRendering
  smEvent
  smSimulators
  )
