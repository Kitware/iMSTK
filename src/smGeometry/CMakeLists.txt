
simmedtk_add_library(smGeometry
  SOURCES
    smMeshModel.cpp
    smPlaneModel.cpp
  PUBLIC_HEADERS
    smMeshModel.h
    smPlaneModel.h
)

target_link_libraries(smGeometry
  PRIVATE
    smCore
    smMesh
    smRendering
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
