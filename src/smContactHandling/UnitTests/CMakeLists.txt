set(Module ContactHandling)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  PenaltyContactHandlingSpec.cpp  
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(${Module}UnitTestRunner 
    smCollision 
    smCore     
    smEvent
    smCollision
    smSimulators
    smUtilities
    smRendering
    smMesh
    smContactHandling)

simple_test(${Module} --reporter=spec)