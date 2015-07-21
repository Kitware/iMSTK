
simmedtk_add_library(VirtualTools
  SOURCES
    curvedGrasper.cpp
  PUBLIC_HEADERS
    curvedGrasper.h
)

target_link_libraries(VirtualTools
  PRIVATE
    Core
    Mesh
    Simulators
    Event
)
