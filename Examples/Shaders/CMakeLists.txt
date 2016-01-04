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
  )

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set(${APP}_DATA "${CMAKE_CURRENT_BINARY_DIR}")
if(NOT EXISTS ${${APP}_DATA})
    file(MAKE_DIRECTORY ${${APP}_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318851/ShadersData.tar)
set(FILE_SHA1 df719673804075b4505ea267b39e5cd68568f72c)
set(LOCAL_FILE ${EXAMPLES_DATA}/ShadersData.tar)

midas_download(${FILE_URL} ${FILE_SHA1} ${LOCAL_FILE})

add_custom_command(
    TARGET ${APP}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${${APP}_DATA}
    COMMENT "-- Unpacking ${LOCAL_FILE}"
)
