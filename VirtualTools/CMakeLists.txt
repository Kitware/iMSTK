
simmedtk_add_library(VirtualTools
  SOURCES
#     CurvedGrasper.cpp
    ToolCoupler.cpp
  PUBLIC_HEADERS
    ToolCoupler.h
#     CurvedGrasper.h
)

target_link_libraries(VirtualTools
  PRIVATE
    Core
    Mesh
    Simulators
    Event
)
