set(Module ContactHandling)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  PenaltyContactHandlingSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(${Module}UnitTestRunner
    Collision
    Core
    Event
    Collision
    Simulators
    Rendering
    Mesh
    ContactHandling)

simple_test(${Module} --reporter=spec)
