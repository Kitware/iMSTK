
add_executable(renderCubeToTexture main.cpp)
target_link_libraries(renderCubeToTexture
  Core
  Event
  Rendering
  Mesh
  Collision
  ExamplesCommon
  Geometry
  )
