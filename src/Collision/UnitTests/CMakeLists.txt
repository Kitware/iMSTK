set(Module Collision)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  SpatialHashSpec.cpp
  OctreeCellSpec.cpp
  MeshToMeshCollisionSpec.cpp
  CollisionPairSpec.cpp
  MeshCollisionModelSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(${Module}UnitTestRunner Geometry Collision Core Rendering Mesh Event)

simple_test(${Module} --reporter=spec)
