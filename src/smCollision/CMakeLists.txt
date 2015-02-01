
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
)

target_link_libraries(smCollision
  PUBLIC
    smCore
)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(smCollision
    PRIVATE
      -std=c++11
  )
endif()
