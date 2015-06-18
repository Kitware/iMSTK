add_executable(CollisionDetectionBVH main.cpp CollisionDetectionBVH.cpp CollisionDetectionBVH.h)

target_link_libraries(CollisionDetectionBVH
  smCollision
  smCore
  smEvent
  smMesh
  smRendering
  smSimulators
  ExamplesCommon
  )
