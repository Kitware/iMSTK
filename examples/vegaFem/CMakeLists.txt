
add_executable(vegaFemExample main.cpp  VegaFemExample.cpp vegaFemExample.h)
target_link_libraries(vegaFemExample
  smCore
  smRendering
  smSimulators
  smMesh
  )

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(vegaFemExample
    PRIVATE
      -std=c++11
  )
endif()
