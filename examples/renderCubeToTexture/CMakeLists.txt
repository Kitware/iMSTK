
add_executable(renderCubeToTexture main.cpp)
target_link_libraries(renderCubeToTexture
  smCore
  smEvent
  smRendering
  smMesh
  smCollision
  ExamplesCommon
  smGeometry
  )
