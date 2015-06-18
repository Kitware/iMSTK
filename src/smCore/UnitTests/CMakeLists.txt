set(Module Core)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  Factory.cpp
)

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)
target_link_libraries(${Module}UnitTestRunner smCore)
simple_test(${Module} --reporter=spec)
