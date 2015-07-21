add_executable(CollisionDetectionBVH main.cpp CollisionDetectionBVH.cpp CollisionDetectionBVH.h)

target_link_libraries(CollisionDetectionBVH
  Collision
  Core
  Event
  Mesh
  Rendering
  Simulators
  ExamplesCommon
  )
