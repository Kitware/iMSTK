
add_executable(renderCube main.cpp  RenderExample.cpp RenderExample.h)
target_link_libraries(renderCube
  smCore
  smRendering
  smShader
  smMesh
  )

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(renderCube BEFORE
    PRIVATE
      -std=c++11
  )
endif()
