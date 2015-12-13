set(Module Solvers)

configure_file(SolversSpec.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/SolversSpec.cpp @ONLY)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  ${CMAKE_CURRENT_BINARY_DIR}/SolversSpec.cpp
)

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)
target_link_libraries(${Module}UnitTestRunner Core Solvers)
simple_test(${Module} --reporter=spec)
