
add_executable(vegaFemExample main.cpp)
target_link_libraries(vegaFemExample
  Core
  Rendering
  RenderDelegates
  Simulators
  Mesh
  Collision
  ContactHandling
  Event
  ExamplesCommon
  )
