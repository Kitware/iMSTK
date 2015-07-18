set(Module Event)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  EventHandlerSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(${Module}UnitTestRunner Collision Core Rendering Mesh Event)

simple_test(${Module} --reporter=spec)
