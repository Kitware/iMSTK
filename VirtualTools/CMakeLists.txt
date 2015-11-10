
include(${VTK_USE_FILE})

simmedtk_add_library(VirtualTools
  SOURCES
#     CurvedGrasper.cpp
    ToolCoupler.cpp
	LaparoscopicCameraController.cpp
  PUBLIC_HEADERS
    ToolCoupler.h
	LaparoscopicCameraController.h
#     CurvedGrasper.h
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
