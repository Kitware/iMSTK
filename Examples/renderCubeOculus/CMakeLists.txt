add_executable(renderCubeOculus main.cpp)
target_link_libraries(renderCubeOculus
  Core
  Rendering
  RenderingOculus
  Mesh
  Event
  Geometry
  ExamplesCommon
  Oculus::Oculus
  )

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(renderCubeOculus BEFORE
    PRIVATE
      -std=c++11
  )
endif()
