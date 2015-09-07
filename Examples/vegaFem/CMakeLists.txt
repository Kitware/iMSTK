add_executable(vegaFemExample main.cpp)
target_link_libraries(vegaFemExample
  Core
  Geometry
  VTKRendering
  Rendering
  RenderDelegates
  Simulators
  Mesh
  Collision
  ContactHandling
  Event
  ExamplesCommon
  IO
)

# Configure an example to run in the build directory:
set(dropbox_files
  dropbox.bou
  dropbox.config
  dropbox.lights
  dropbox.mtl
  dropbox.obj
  dropbox.veg
)

set(SimMedTK_VEGA_EXAMPLE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
foreach (_file ${dropbox_files})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/${_file}.in
    ${CMAKE_CURRENT_BINARY_DIR}/dropbox/${_file}
    @ONLY
  )
endforeach()
# Generate the mass-matrix and interpolation simulation
# inputs from the volume mesh and surface mesh.
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mass"
  COMMAND
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generateMassMatrix"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mass"
  MAIN_DEPENDENCY
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
  COMMENT "Generating mass matrix for dropbox example."
)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.interp"
  COMMAND
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generateInterpolant"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.obj"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.interp"
  MAIN_DEPENDENCY
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.obj"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mtl"
  COMMENT "Generating surface interpolation weights for dropbox example."
)
add_custom_target(dropbox_aux ALL
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.interp"
    "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mass"
)
