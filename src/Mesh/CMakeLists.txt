
simmedtk_add_library(Mesh
  SOURCES
    ImportExport.cpp
    Mesh.cpp
    SurfaceMesh.cpp
    VolumeMesh.cpp
    Lattice.cpp
    VegaSceneObject.cpp
    VegaSceneObjectDeformable.cpp
    VegaSceneObjectWithRestPosition.cpp
    VegaVolumetricMesh.cpp
  PUBLIC_HEADERS
    ImportExport.h
    Mesh.h
    SurfaceMesh.h
    VolumeMesh.h
    Lattice.h
    LatticeTypes.h
    VegaSceneObject.h
    VegaSceneObjectDeformable.h
    VegaSceneObjectWithRestPosition.h
)

target_link_libraries(Mesh
  PRIVATE
    Core
    Rendering
  PUBLIC
    Assimp::Assimp
    VegaFEM::minivector
    VegaFEM::volumetricMesh
    )

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
