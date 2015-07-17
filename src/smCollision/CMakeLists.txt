
simmedtk_add_library(smCollision
  SOURCES
    smSpatialHashCollision.cpp
    smOctreeCell.cpp
    smCollisionMoller.cpp
    smCollisionUtilities.cpp
    smOctreeCell.cpp
    smMeshCollisionModel.cpp
    smMeshToMeshCollision.cpp
    smPlaneToMeshCollision.cpp
    smPlaneCollisionModel.cpp
  PUBLIC_HEADERS
    smSpatialGrid.h
    smSpatialHash.h
    smOctreeCell.h
    smCollisionMoller.h
    smMeshCollisionModel.h
    smMeshToMeshCollision.h
    smPlaneToMeshCollision.h
)

target_link_libraries(smCollision
  PRIVATE
    smCore
    smEvent
    smMesh
    smGeometry
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
