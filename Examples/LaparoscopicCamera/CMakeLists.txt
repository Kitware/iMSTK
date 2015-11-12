set(APP LapCameraNavigationApp)

add_executable(${APP} main.cpp)
target_link_libraries(${APP}
  Core
  Geometry
  VTKRendering
  Simulators
  Mesh
  Event
  IO
  Devices
  VirtualTools
)

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set(${APP}_DATA "${CMAKE_CURRENT_BINARY_DIR}")
if(NOT EXISTS ${${APP}_DATA})
    file(MAKE_DIRECTORY ${${APP}_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318829/CameraNavAppData.tar)
set(FILE_SHA1 da39a3ee5e6b4b0d3255bfef95601890afd80709)
set(LOCAL_FILE ${EXAMPLES_DATA}/CameraNavAppData.tar)


midas_download(${FILE_URL} ${FILE_SHA1} ${LOCAL_FILE})

# Extract data
set(DATA
    ${${APP}_DATA}/Target-marker.vtk
    ${${APP}_DATA}/Target.vtk
)

add_custom_command(
    TARGET ${APP}
    POST_BUILD
#     BYPRODUCTS ${DATA}
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${${APP}_DATA}
    COMMENT "Unpacking ${LOCAL_FILE}"
)
