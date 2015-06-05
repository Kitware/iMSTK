
add_executable(vegaFemExample main.cpp)
target_link_libraries(vegaFemExample
  smCore
  smRendering
  smSimulators
  smMesh
  smCollision
  smContactHandling
  smShader
  smEvent
  ExamplesCommon
  )
