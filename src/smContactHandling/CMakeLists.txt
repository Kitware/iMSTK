
simmedtk_add_library(smContactHandling
  SOURCES
    smSpatialHashCollision.cpp
    smOctreeCell.cpp
    smCollisionMoller.cpp
    smCollisionUtilities.cpp
    smCollisionConfig.cpp
    smOctreeCell.cpp
    smCollisionDetection.cpp
    smCollisionPair.cpp
    smMeshToMeshCollision.cpp
    smPenaltyContactHandling.cpp
    smContactHandling.cpp
	smPBDSceneObject.cpp
	smFemSceneObject.cpp
	smVegaFemSceneObject.cpp
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
	${CMAKE_SOURCE_DIR}/include/smContactHandling/smContactConfig.h
    ${CMAKE_SOURCE_DIR}/include/smContactHandling/smPenaltyContactHandling.h
    ${CMAKE_SOURCE_DIR}/include/smContactHandling/smContactHandling.h	
	${CMAKE_SOURCE_DIR}/include/smSimulators/smSceneObject.h
	${CMAKE_SOURCE_DIR}/include/smSimulators/smPBDSceneObject.h
	${CMAKE_SOURCE_DIR}/include/smSimulators/smFemSceneObject.h
	${CMAKE_SOURCE_DIR}/include/smSimulators/smVegaFemSceneObject.h
)

target_link_libraries(smContactHandling
  PUBLIC
    smCore
  PRIVATE
    smEvent
)

if(BUILD_TESTING)
# add tests here
endif()
