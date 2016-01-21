set(APP LaparoscopicCamera)

add_executable(${APP} main.cpp)
target_link_libraries(${APP}
  Core
  Geometry
  VTKRendering
  Simulators
  Mesh
  Collision
  Event
  IO
  Devices
  VirtualTools
  Solvers
  SceneModels
  SimulationManager
)

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
