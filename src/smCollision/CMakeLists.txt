
simmedtk_add_library(smCollision
  SOURCES
    smSpatialHash.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionConfig.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionDetection.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smCollisionModel.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smPQP.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smSpatialGrid.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smSpatialHash.h
    ${CMAKE_SOURCE_DIR}/include/smCollision/smOctreeCell.h
)

target_link_libraries(smCollision
  PUBLIC
    smCore
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
