
simmedtk_add_library(Collision
  SOURCES
    SpatialHashCollision.cpp
    OctreeCell.cpp
    CollisionMoller.cpp
    CollisionUtilities.cpp
    OctreeCell.cpp
    MeshCollisionModel.cpp
    MeshToMeshCollision.cpp
    PlaneToMeshCollision.cpp
    PlaneCollisionModel.cpp
  PUBLIC_HEADERS
    SpatialGrid.h
    SpatialHash.h
    OctreeCell.h
    CollisionMoller.h
    MeshCollisionModel.h
    MeshToMeshCollision.h
    PlaneToMeshCollision.h
)

target_link_libraries(Collision
  PRIVATE
    Core
    Event
    Mesh
    Geometry
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
