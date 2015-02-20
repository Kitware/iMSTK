
add_executable(renderCube main.cpp  RenderCube.cpp RenderCube.h)
target_link_libraries(renderCube
  smCore
  smRendering
  smShader
  smMesh
  )
