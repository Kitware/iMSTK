
add_executable(renderCubeOculus main.cpp  RenderCubeOculus.cpp RenderCubeOculus.h)
target_link_libraries(renderCubeOculus
  smCore
  smRendering
  smShader
  smMesh
  )

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(renderCubeOculus BEFORE
    PRIVATE
      -std=c++11
  )
endif()
