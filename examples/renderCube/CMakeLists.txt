
add_executable(renderCube main.cpp)
target_link_libraries(renderCube
  smCore
  smRendering
  smRenderDelegates
  smMesh
  smEvent
  smCollision
  ExamplesCommon
  smGeometry
  )
