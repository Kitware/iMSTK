
simmedtk_add_library(VirtualTools
  SOURCES
#     CurvedGrasper.cpp
    ToolCoupler.cpp
	LaparoscopicCameraCoupler.cpp
  PUBLIC_HEADERS
    ToolCoupler.h
	LaparoscopicCameraCoupler.h
#     CurvedGrasper.h
)

target_link_libraries(VirtualTools
  PRIVATE
    Core
    Mesh
    Event
    Devices
)
