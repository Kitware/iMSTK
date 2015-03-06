
add_executable(renderCube main.cpp  RenderCube.cpp RenderCube.h)
target_link_libraries(renderCube
  smCore
  smRendering
  smShader
  smMesh
  sfml::system
  sfml::audio
  sfml::graphics
  sfml::network
  sfml::window
  )
