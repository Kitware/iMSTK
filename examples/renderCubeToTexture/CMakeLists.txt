
add_executable(renderCubeToTexture main.cpp  RenderCube.cpp RenderCube.h)
target_link_libraries(renderCubeToTexture
  smCore
  smRendering
  smShader
  smMesh
  )
