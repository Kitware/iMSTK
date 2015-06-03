set(Module Collision)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  SpatialHashSpec.cpp
  OctreeCellSpec.cpp
  MeshToMeshCollisionSpec.cpp
  CollisionPairSpec.cpp
  MeshCollisionModelSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(${Module}UnitTestRunner smGeometry smCollision smCore smRendering smUtilities smMesh smEvent)

simple_test(${Module} --reporter=spec)
