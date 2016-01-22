set(Module Mesh)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  VegaVolumetricMeshSpec.cpp
)

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)
target_link_libraries(${Module}UnitTestRunner Core Mesh IO)
simple_test(${Module} --reporter=xunit)

set(SOURCE_PATH ${PROJECT_SOURCE_DIR}/Testing/Models)
set(BINARY_PATH ${PROJECT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/Testing/path.in ${CMAKE_CURRENT_BINARY_DIR}/${Module}Config.paths @ONLY)
