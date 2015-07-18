
simmedtk_add_library(Tools
  SOURCES
    curvedGrasper.cpp
  PUBLIC_HEADERS
    curvedGrasper.h
)

target_link_libraries(Tools
  PRIVATE
    Core
    Mesh
    Simulators
    Event
)
