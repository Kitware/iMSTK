
add_executable(vrpnPhantomTest main.cpp)
target_link_libraries(vrpnPhantomTest
  Core
  VTKRendering
  Rendering
  RenderDelegates
  IO
  Mesh
  Event
  Collision
  ExamplesCommon
  Geometry
  Devices
  VirtualTools
  )
