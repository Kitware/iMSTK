
simmedtk_add_library(smCollision
  SOURCES
    smSpatialHashCollision.cpp
    smOctreeCell.cpp
    smCollisionMoller.cpp
    smCollisionUtilities.cpp
    smCollisionConfig.cpp
    smOctreeCell.cpp
    smCollisionDetection.cpp
    smCollisionPair.cpp
    smMeshCollisionModel.cpp
    smMeshToMeshCollision.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionConfig.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionDetection.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionModel.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smSpatialGrid.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smSpatialHash.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smOctreeCell.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionMoller.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionPair.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smMeshCollisionModel.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smMeshToMeshCollision.h
)

target_link_libraries(smCollision
  PUBLIC
    smCore
  PRIVATE
    smEvent
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
