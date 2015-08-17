set(Module IO)

configure_file(IOMeshSpec.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/IOMeshSpec.cpp @ONLY)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  ${CMAKE_CURRENT_BINARY_DIR}/IOMeshSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(${Module}UnitTestRunner Core Mesh IO)

simple_test(${Module} --reporter=spec)

set(VOLUME_MESH_SAMPLE
"# Vega mesh file.
# 4 vertices, 1 elements

*VERTICES
4 3 0 0
1 -2.44627 -0.903874999999999 -1.711465
2 -2.008655 -0.762779999999999 -1.63081
3 -2.248035 -0.599385 -1.41836
4 -2.2993347930303 -1.06841207461711 -0.999830245926915

*ELEMENTS
TET
1 4 0
1 1 2 3 4

*MATERIAL defaultMaterial
ENU, 1000, 1000000000, 0.45

*SET defaultSet
1,

*MATERIAL BODY
ENU, 1000,
allElements, BODY10000000, 0.4

*REGION
allElements, BODY")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SampleMesh.veg ${VOLUME_MESH_SAMPLE})
