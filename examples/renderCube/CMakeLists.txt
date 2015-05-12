
add_executable(renderCube main.cpp)
target_link_libraries(renderCube
  smCore
  smRendering
  smShader
  smMesh
  smEvent
  ExamplesCommon
  sfml::system
  sfml::audio
  sfml::graphics
  sfml::network
  sfml::window
  )
