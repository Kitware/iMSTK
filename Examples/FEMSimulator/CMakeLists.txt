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
)

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set(${APP}_DATA "${CMAKE_CURRENT_BINARY_DIR}")
if(NOT EXISTS ${${APP}_DATA})
    file(MAKE_DIRECTORY ${${APP}_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318708/box.tar)
set(FILE_SHA1 928294a91477bc5d6fa494afb34016ba7f0b40c5)
set(LOCAL_FILE ${EXAMPLES_DATA}/box.tar)

midas_download(${FILE_URL} ${FILE_SHA1} ${LOCAL_FILE})

# Extract data
set(DATA
    ${${APP}_DATA}/box.bou
    ${${APP}_DATA}/box.config
    ${${APP}_DATA}/box.interp
    ${${APP}_DATA}/box.mass
    ${${APP}_DATA}/box.vtk
    ${${APP}_DATA}/box.veg
)

add_custom_command(
    TARGET ${APP}
    POST_BUILD
#     BYPRODUCTS ${DATA}
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${${APP}_DATA}
    COMMENT "Unpacking ${LOCAL_FILE}"
)

# Configure an example to run in the build directory:
# set(dropbox_files
#   dropbox.bou
#   dropbox.config
#   dropbox.lights
#   dropbox.mtl
#   dropbox.vtk
#   dropbox.veg
# )

# set(SimMedTK_VEGA_EXAMPLE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
# foreach (_file ${dropbox_files})
#   configure_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/${_file}.in
#     ${CMAKE_CURRENT_BINARY_DIR}/dropbox/${_file}
#     @ONLY
#   )
# endforeach()
# Generate the mass-matrix and interpolation simulation
# inputs from the volume mesh and surface mesh.
# add_custom_command(
#   OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mass"
#   COMMAND
#     "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generateMassMatrix"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mass"
#   MAIN_DEPENDENCY
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
#   COMMENT "Generating mass matrix for dropbox example."
# )
# add_custom_command(
#   OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.interp"
#   COMMAND
#     "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generateInterpolant"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.obj"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.interp"
#   MAIN_DEPENDENCY
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.veg"
#   DEPENDS
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.obj"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mtl"
#   COMMENT "Generating surface interpolation weights for dropbox example."
# )
# add_custom_target(dropbox_aux ALL
#   DEPENDS
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.interp"
#     "${CMAKE_CURRENT_BINARY_DIR}/dropbox/dropbox.mass"
# )
