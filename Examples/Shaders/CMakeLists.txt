set(APP Shaders)

include(${VTK_USE_FILE})

add_executable(${APP} main.cpp)
target_link_libraries(${APP}
  Core
  Geometry
  VTKRendering
  Rendering
  Mesh
  Event
  IO
  Solvers
  SceneModels
  SimulationManager
  )

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
