
simmedtk_add_library(smMesh
  SOURCES
    smImportExport.cpp
    smMesh.cpp
    smSurfaceMesh.cpp
    smVolumeMesh.cpp
    smLattice.cpp
    smVegaSceneObject.cpp
    smVegaSceneObjectDeformable.cpp
    smVegaSceneObjectWithRestPosition.cpp
    smVegaVolumetricMesh.cpp
  PUBLIC_HEADERS
    smImportExport.h
    smMesh.h
    smSurfaceMesh.h
    smVolumeMesh.h
    smLattice.h
    smLatticeTypes.h
    smVegaSceneObject.h
    smVegaSceneObjectDeformable.h
    smVegaSceneObjectWithRestPosition.h
)

target_link_libraries(smMesh
  PRIVATE
    smCore
    smRendering
  PUBLIC
    Assimp::Assimp
    VegaFEM::minivector
    VegaFEM::volumetricMesh
    )

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
