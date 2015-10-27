
add_executable(renderCubeToTexture main.cpp)
target_link_libraries(renderCubeToTexture
  Core
  Rendering
  RenderDelegates
  IO
  Mesh
  Event
  Collision
  ExamplesCommon
  Geometry
  )
