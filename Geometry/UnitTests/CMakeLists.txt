set(Module Geometry)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  MeshModelSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(${Module}UnitTestRunner Geometry Core Mesh)

simple_test(${Module} --reporter=spec)

set(TEST_SURFACE_MESH

)
