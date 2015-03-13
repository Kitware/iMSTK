
simmedtk_add_library(smMesh
  SOURCES
    smImportExport.cpp
    smMesh.cpp
    smSurfaceMesh.cpp
    smVolumeMesh.cpp
    smLattice.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smMesh/smImportExport.h
    ${CMAKE_SOURCE_DIR}/include/smMesh/smMesh.h
    ${CMAKE_SOURCE_DIR}/include/smMesh/smSurfaceMesh.h
    ${CMAKE_SOURCE_DIR}/include/smMesh/smVolumeMesh.h
    ${CMAKE_SOURCE_DIR}/include/smMesh/smLattice.h
    ${CMAKE_SOURCE_DIR}/include/smMesh/smLatticeTypes.h
)

target_link_libraries(smMesh
  PUBLIC
    Assimp::Assimp
  PRIVATE
    smCore
    smRendering
    smCollision
    )
