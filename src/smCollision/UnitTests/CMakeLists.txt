set(Module Collision)

add_executable(${Module}UnitTestRunner 
  ${BANDIT_RUNNER} 
  SpatialHashSpec.cpp
  OctreeCellSpec.cpp
  )

target_link_libraries(${Module}UnitTestRunner smCollision smCore smRendering smUtilities smMesh)

simple_test(${Module} --reporter=spec)
