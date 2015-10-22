add_executable(renderCubeOculus main.cpp)
target_link_libraries(renderCubeOculus
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