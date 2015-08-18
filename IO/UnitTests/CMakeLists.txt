set(Module IO)

configure_file(IOMeshSpec.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/IOMeshSpec.cpp @ONLY)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  ${CMAKE_CURRENT_BINARY_DIR}/IOMeshSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(${Module}UnitTestRunner Core Mesh IO)

simple_test(${Module} --reporter=spec)
