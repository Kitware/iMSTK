
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
    ${CMAKE_SOURCE_DIR}/include/smCollision/smSpatialGrid.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smSpatialHash.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smOctreeCell.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionMoller.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smMeshCollisionModel.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smMeshToMeshCollision.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smPlaneToMeshCollision.h
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
