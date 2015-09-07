
add_executable(vegaFemExample main.cpp)
target_link_libraries(vegaFemExample
  Core
  Geometry
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
