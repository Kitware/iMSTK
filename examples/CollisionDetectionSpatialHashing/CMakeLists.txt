add_executable(CollisionDetectionSpatialHashing main.cpp CollisionDetectionSpatialHashing.cpp CollisionDetectionSpatialHashing.h)

target_link_libraries(CollisionDetectionSpatialHashing
  Core
  Rendering
  Mesh
  Simulators
  Collision
  Event
  ExamplesCommon
  )
