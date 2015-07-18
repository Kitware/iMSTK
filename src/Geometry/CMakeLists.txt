
simmedtk_add_library(Geometry
  SOURCES
    MeshModel.cpp
    PlaneModel.cpp
  PUBLIC_HEADERS
    MeshModel.h
    PlaneModel.h
)

target_link_libraries(Geometry
  PRIVATE
    Core
    Mesh
    Rendering
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
