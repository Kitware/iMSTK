add_executable(renderCubeToTextureOculus main.cpp)
target_link_libraries(renderCubeToTextureOculus
  Core
  Event
  Rendering
  RenderingOculus
  Mesh
  Geometry
  ExamplesCommon
  Oculus::Oculus
  )
