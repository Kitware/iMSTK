set(Module CollisionContext)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  CollisionContextSpec.cpp
)
target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)
target_link_libraries(${Module}UnitTestRunner
    CollisionContext
    Event
    Collision
    Simulators
    ContactHandling)
simple_test(${Module} --reporter=xunit)
