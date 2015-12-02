
simmedtk_add_library(Collision
  SOURCES
    CollisionMoller.cpp
    MeshCollisionModel.cpp
    MeshToMeshCollision.cpp
    OctreeCell.cpp
    PlaneCollisionModel.cpp
    PlaneToMeshCollision.cpp
    SpatialHashCollision.cpp
  PUBLIC_HEADERS
    CollisionMoller.h
    MeshCollisionModel.h
    MeshToMeshCollision.h
    OctreeCell.h
    PlaneCollisionModel.h
    PlaneToMeshCollision.h
    SpatialHashCollision.h
    SurfaceTree.h
    SurfaceTree.hpp
    SurfaceTreeCell.h
    SurfaceTreeIterator.h
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
