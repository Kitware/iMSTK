
simmedtk_add_library(IO
  SOURCES
    IOMesh.cpp
    IOMeshAssimpDelegate.cpp
    IOMeshVegaDelegate.cpp
    IOMeshVTKDelegate.cpp
  PUBLIC_HEADERS
    Mesh.h
    SurfaceMesh.h
    VolumeMesh.h
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
