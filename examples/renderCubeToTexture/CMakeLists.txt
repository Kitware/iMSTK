
add_executable(renderCubeToTexture main.cpp)
target_link_libraries(renderCubeToTexture
  smCore
  smEvent
  smRendering
  smShader
  smMesh
  smCollision
  ExamplesCommon
  )
