include(${VTK_USE_FILE})
add_executable(renderCube main.cpp)
target_link_libraries(renderCube
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
  )
