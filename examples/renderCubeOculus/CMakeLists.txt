add_executable(renderCubeOculus main.cpp)
target_link_libraries(renderCubeOculus
  smCore
  smRendering
  smRenderingOculus
  smShader
  smMesh
  smEvent
  ExamplesCommon
  Oculus::Oculus
  )

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(renderCubeOculus BEFORE
    PRIVATE
      -std=c++11
  )
endif()
