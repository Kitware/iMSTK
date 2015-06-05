
add_executable(renderCube main.cpp)
target_link_libraries(renderCube
  smCore
  smRendering
  smShader
  smMesh
  smEvent
  smCollision
  ExamplesCommon
  smGeometry
  )
