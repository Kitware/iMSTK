
add_executable(AudioExample main.cpp  AudioExample.cpp AudioExample.h)
target_link_libraries(AudioExample
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
