add_executable(CollisionDetectionSpatialHashing main.cpp CollisionDetectionSpatialHashing.cpp CollisionDetectionSpatialHashing.h)

target_link_libraries(CollisionDetectionSpatialHashing
  smCore
  smRendering
  smShader
  smMesh
  smSimulators
  smCollision
  smEvent
  ExamplesCommon
  )
