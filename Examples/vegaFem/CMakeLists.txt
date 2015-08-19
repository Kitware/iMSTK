
add_executable(vegaFemExample main.cpp)
target_link_libraries(vegaFemExample
  Core
  VtkRendering
  Rendering
  RenderDelegates
  Simulators
  Mesh
  Collision
  ContactHandling
  Event
  ExamplesCommon
  )
