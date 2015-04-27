add_executable(CollisionDetectionBVH main.cpp CollisionDetectionBVH.cpp CollisionDetectionBVH.h)

target_link_libraries(CollisionDetectionBVH
  smCore
  smRendering
  smShader
  smMesh
  smSimulators
  smCollision
  )
