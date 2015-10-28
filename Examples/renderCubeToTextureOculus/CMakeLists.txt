add_executable(renderCubeToTextureOculus main.cpp)
target_link_libraries(renderCubeToTextureOculus
  Core
  Rendering
  RenderingOculus
  RenderDelegates
  IO
  Mesh
  Event
  Collision
  ExamplesCommon
  Geometry
  Oculus::Oculus
  )
