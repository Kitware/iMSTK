set(APP FEMSimulator)

add_executable(${APP} main.cpp)
target_link_libraries(${APP}
  Core
  Geometry
  VTKRendering
  Simulators
  Mesh
  Collision
  ContactHandling
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
