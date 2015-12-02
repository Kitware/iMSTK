
include(${VTK_USE_FILE})

simmedtk_add_library(VirtualTools
  SOURCES
    ToolCoupler.cpp
	LaparoscopicCameraController.cpp
  PUBLIC_HEADERS
    ToolCoupler.h
	LaparoscopicCameraController.h
)

target_link_libraries(VirtualTools
  PRIVATE
    Core
    Mesh
    Event
    Devices
  PUBLIC
    ${VTK_LIBRARIES}
)
