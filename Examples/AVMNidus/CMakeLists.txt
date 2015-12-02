set(APP AVMNidus)

add_executable(${APP} main.cpp)
target_link_libraries(${APP}
  Core
  VTKRendering
  Rendering
  RenderDelegates
  Simulators
  Mesh
  Collision
  ContactHandling
  Event
  IO
  )

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set(${APP}_DATA "${CMAKE_CURRENT_BINARY_DIR}")
if(NOT EXISTS ${${APP}_DATA})
    file(MAKE_DIRECTORY ${${APP}_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318709/nidus.tar)
set(FILE_SHA1 40206fc9b45b9bc32ebda92c64d1553038ab7646)
set(LOCAL_FILE ${EXAMPLES_DATA}/nidus.tar)

midas_download(${FILE_URL} ${FILE_SHA1} ${LOCAL_FILE})

# Extract data
set(DATA
    ${${APP}_DATA}/nidus-empty.bou
    ${${APP}_DATA}/nidus.config
    ${${APP}_DATA}/nidus1764.interp
    ${${APP}_DATA}/nidus1764.mass
    ${${APP}_DATA}/nidusS7415.vtk
    ${${APP}_DATA}/nidusV1764.vtk
    ${${APP}_DATA}/wet_frag.glsl
    ${${APP}_DATA}/wet_vert.glsl
)

add_custom_command(
    TARGET ${APP}
    POST_BUILD
#     BYPRODUCTS ${DATA}
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${${APP}_DATA}
    COMMENT "-- Unpacking ${LOCAL_FILE}"
)
