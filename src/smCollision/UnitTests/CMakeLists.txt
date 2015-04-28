set(Module Collision)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  SpatialHashSpec.cpp
  OctreeCellSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(${Module}UnitTestRunner smCollision smCore smRendering smUtilities smMesh)

simple_test(${Module} --reporter=spec)
