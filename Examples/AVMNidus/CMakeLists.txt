
add_executable(AVMNidus main.cpp)
target_link_libraries(AVMNidus
  Core
  VTKRendering
  Rendering
  RenderDelegates
  Simulators
  Mesh
  Collision
  ContactHandling
  Event
  ExamplesCommon
  IO
  )
