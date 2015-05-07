
add_executable(renderCubeToTextureOculus main.cpp  RenderCube.cpp RenderCube.h)
target_link_libraries(renderCubeToTextureOculus
  smCore
  smRendering
  smRenderingOculus
  smShader
  smMesh
  Oculus::Oculus
  )
