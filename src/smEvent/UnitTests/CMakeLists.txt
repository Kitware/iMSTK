set(Module Event)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  EventSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

simple_test(${Module} --reporter=spec)
