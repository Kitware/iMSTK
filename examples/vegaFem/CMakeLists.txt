
add_executable(vegaFemExample main.cpp)
target_link_libraries(vegaFemExample
  smCore
  smRendering
  smRenderDelegates
  smSimulators
  smMesh
  smCollision
  smContactHandling
  smEvent
  ExamplesCommon
  )
