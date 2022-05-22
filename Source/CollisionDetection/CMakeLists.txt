#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( CollisionDetection
  H_FILES
    CollisionDetection/imstkBidirectionalPlaneToSphereCD.h
    CollisionDetection/imstkImplicitGeometryToPointSetCCD.h
    CollisionDetection/imstkImplicitGeometryToPointSetCD.h
    CollisionDetection/imstkMeshToMeshBruteForceCD.h
    CollisionDetection/imstkPointSetToCapsuleCD.h
    CollisionDetection/imstkPointSetToCylinderCD.h
    CollisionDetection/imstkPointSetToOrientedBoxCD.h
    CollisionDetection/imstkPointSetToPlaneCD.h
    CollisionDetection/imstkPointSetToSphereCD.h
    CollisionDetection/imstkSphereToCylinderCD.h
    CollisionDetection/imstkSphereToSphereCD.h
    CollisionDetection/imstkSurfaceMeshToCapsuleCD.h
    CollisionDetection/imstkSurfaceMeshToSphereCD.h
    CollisionDetection/imstkSurfaceMeshToSurfaceMeshCD.h
    CollisionDetection/imstkTetraToLineMeshCD.h
    CollisionDetection/imstkTetraToPointSetCD.h
    CollisionDetection/imstkUnidirectionalPlaneToSphereCD.h
    imstkCDObjectFactory.h
    imstkCollisionData.h
    imstkCollisionDetectionAlgorithm.h
    imstkCollisionUtils.h
    Picking/imstkCellPicker.h
    Picking/imstkPickingAlgorithm.h
    Picking/imstkPointPicker.h  
    Picking/imstkVertexPicker.h
  CPP_FILES
    CollisionDetection/imstkBidirectionalPlaneToSphereCD.cpp
    CollisionDetection/imstkImplicitGeometryToPointSetCCD.cpp
    CollisionDetection/imstkImplicitGeometryToPointSetCD.cpp
    CollisionDetection/imstkMeshToMeshBruteForceCD.cpp
    CollisionDetection/imstkPointSetToCapsuleCD.cpp
    CollisionDetection/imstkPointSetToCylinderCD.cpp
    CollisionDetection/imstkPointSetToOrientedBoxCD.cpp
    CollisionDetection/imstkPointSetToPlaneCD.cpp
    CollisionDetection/imstkPointSetToSphereCD.cpp
    CollisionDetection/imstkSphereToCylinderCD.cpp
    CollisionDetection/imstkSphereToSphereCD.cpp
    CollisionDetection/imstkSurfaceMeshToCapsuleCD.cpp
    CollisionDetection/imstkSurfaceMeshToSphereCD.cpp
    CollisionDetection/imstkSurfaceMeshToSurfaceMeshCD.cpp
    CollisionDetection/imstkTetraToLineMeshCD.cpp
    CollisionDetection/imstkTetraToPointSetCD.cpp
    CollisionDetection/imstkUnidirectionalPlaneToSphereCD.cpp
    imstkCDObjectFactory.cpp
    imstkCollisionDetectionAlgorithm.cpp
    imstkCollisionUtils.cpp
    Picking/imstkCellPicker.cpp
    Picking/imstkPointPicker.cpp
    Picking/imstkVertexPicker.cpp
  DEPENDS
    FilteringCore
    #fcl
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
  add_subdirectory(VisualTesting)
endif()