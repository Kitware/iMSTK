add_executable(renderCubeToTextureOculus main.cpp)
target_link_libraries(renderCubeToTextureOculus
  smCore
  smEvent
  smRendering
  smRenderingOculus
  smShader
  smMesh
  smGeometry
  ExamplesCommon
  Oculus::Oculus
  )
