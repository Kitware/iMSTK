
simmedtk_add_library(Mesh
  SOURCES
    Mesh.cpp
    SurfaceMesh.cpp
    VolumeMesh.cpp
    VegaSceneObject.cpp
    VegaSceneObjectDeformable.cpp
    VegaSceneObjectWithRestPosition.cpp
    VegaVolumetricMesh.cpp
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
