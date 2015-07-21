
simmedtk_add_library(VirtualTools
  SOURCES
    CurvedGrasper.cpp
  PUBLIC_HEADERS
    CurvedGrasper.h
)

target_link_libraries(VirtualTools
  PRIVATE
    Core
    Mesh
    Simulators
    Event
)
