
add_executable(renderCube main.cpp)
target_link_libraries(renderCube
  Core
  Rendering
  RenderDelegates
  Mesh
  Event
  Collision
  ExamplesCommon
  Geometry
  )
