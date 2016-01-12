simmedtk_add_library(Mesh
  SOURCES
    SurfaceMesh.cpp
    VegaVolumetricMesh.cpp
  PUBLIC_HEADERS
    SurfaceMesh.h
    VegaVolumetricMesh.h
)

target_link_libraries(Mesh
  PRIVATE
    Core
  PUBLIC
    Assimp::Assimp
    VegaFEM::minivector
    VegaFEM::volumetricMesh
    )

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
