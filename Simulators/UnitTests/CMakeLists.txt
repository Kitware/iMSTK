set(Module Simulators)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  VegaFEMDeformableSceneObjectSpec.cpp
)

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)
target_link_libraries(${Module}UnitTestRunner Core Mesh IO Solvers TimeIntegrators Simulators)
simple_test(${Module} --reporter=xunit)
