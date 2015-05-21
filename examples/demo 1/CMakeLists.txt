
add_executable(vegaFemExample main.cpp  demo1.cpp demo1.h)
target_link_libraries(demo1
  smCore
  smRendering
  smSimulators
  smMesh
  smShader
  )
