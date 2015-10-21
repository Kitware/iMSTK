set(ExamplesCommon_sources)
set(ExamplesCommon_headers)
set(ExamplesCommon_link_libs)

list(APPEND ExamplesCommon_sources
  wasdCameraController.cpp
  KeyPressSDKShutdown.cpp
  hapticController.cpp
  pzrMouseCameraController.cpp
  ExampleCube.cpp
)

list(APPEND ExamplesCommon_headers
  wasdCameraController.h
  KeyPressSDKShutdown.h
  hapticController.h
  pzrMouseCameraController.h
  ExampleCube.h
)

list(APPEND ExamplesCommon_link_libs
  Core
  Rendering
  Event
  Simulators
)

if(SimMedTK_USE_PHANTOM_OMNI)
  list(APPEND ExamplesCommon_sources
    vrpnPhantomObjectController.cpp
  )
  list(APPEND ExamplesCommon_headers
    vrpnPhantomObjectController.h
  )
  list(APPEND ExamplesCommon_link_libs
    Devices
  )
endif(SimMedTK_USE_PHANTOM_OMNI)

simmedtk_add_library(ExamplesCommon
  SOURCES
    ${ExamplesCommon_sources}

  PUBLIC_HEADERS
    ${ExamplesCommon_headers}
)

target_link_libraries(ExamplesCommon
  ${ExamplesCommon_link_libs}
)
