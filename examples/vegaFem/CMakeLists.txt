
add_executable(vegaFemExample main.cpp  VegaFemExample.cpp vegaFemExample.h)
target_link_libraries(vegaFemExample
  smCore
  smRendering
  smSimulators
  smMesh
  smShader
  )
