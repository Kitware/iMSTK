
add_executable(renderCube main.cpp  RenderExample.cpp RenderExample.h)
target_link_libraries(renderCube
  smCore
  smRendering
  smShader
  smMesh
  )
