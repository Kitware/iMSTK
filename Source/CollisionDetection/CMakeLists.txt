#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( CollisionDetection
  H_FILES
    CollisionDetection/imstkBidirectionalPlaneToSphereCD.h
    CollisionDetection/imstkCompoundCD.h
    CollisionDetection/imstkCapsuleToCapsuleCD.h
    CollisionDetection/imstkClosedSurfaceMeshToMeshCD.h
    CollisionDetection/imstkClosedSurfaceMeshToCapsuleCD.h
    CollisionDetection/imstkEdgeEdgeCCDState.h
    CollisionDetection/imstkImplicitGeometryToPointSetCCD.h
    CollisionDetection/imstkImplicitGeometryToPointSetCD.h
    CollisionDetection/imstkLineMeshToLineMeshCCD.h
    CollisionDetection/imstkPointSetToCapsuleCD.h
    CollisionDetection/imstkPointSetToCylinderCD.h
    CollisionDetection/imstkPointSetToOrientedBoxCD.h
    CollisionDetection/imstkPointSetToPlaneCD.h
    CollisionDetection/imstkPointSetToSphereCD.h
    CollisionDetection/imstkSphereToCapsuleCD.h
    CollisionDetection/imstkSphereToCylinderCD.h
    CollisionDetection/imstkSphereToSphereCD.h
    CollisionDetection/imstkSurfaceMeshToCapsuleCD.h
    CollisionDetection/imstkSurfaceMeshToSphereCD.h
    CollisionDetection/imstkSurfaceMeshToSurfaceMeshCD.h
    CollisionDetection/imstkTetraToLineMeshCD.h
    CollisionDetection/imstkTetraToPointSetCD.h
    CollisionDetection/imstkUnidirectionalPlaneToCapsuleCD.h
    CollisionDetection/imstkUnidirectionalPlaneToSphereCD.h
    CollisionDetection/imstkLineMeshToSphereCD.h
    CollisionDetection/imstkLineMeshToCapsuleCD.h
    imstkCCDAlgorithm.h
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
    CollisionDetection/imstkCompoundCD.cpp
    CollisionDetection/imstkCapsuleToCapsuleCD.cpp
    CollisionDetection/imstkClosedSurfaceMeshToMeshCD.cpp
    CollisionDetection/imstkClosedSurfaceMeshToCapsuleCD.cpp
    CollisionDetection/imstkEdgeEdgeCCDState.cpp
    CollisionDetection/imstkImplicitGeometryToPointSetCCD.cpp
    CollisionDetection/imstkImplicitGeometryToPointSetCD.cpp
    CollisionDetection/imstkLineMeshToLineMeshCCD.cpp
    CollisionDetection/imstkPointSetToCapsuleCD.cpp
    CollisionDetection/imstkPointSetToCylinderCD.cpp
    CollisionDetection/imstkPointSetToOrientedBoxCD.cpp
    CollisionDetection/imstkPointSetToPlaneCD.cpp
    CollisionDetection/imstkPointSetToSphereCD.cpp
    CollisionDetection/imstkSphereToCapsuleCD.cpp
    CollisionDetection/imstkSphereToCylinderCD.cpp
    CollisionDetection/imstkSphereToSphereCD.cpp
    CollisionDetection/imstkSurfaceMeshToCapsuleCD.cpp
    CollisionDetection/imstkSurfaceMeshToSphereCD.cpp
    CollisionDetection/imstkSurfaceMeshToSurfaceMeshCD.cpp
    CollisionDetection/imstkTetraToLineMeshCD.cpp
    CollisionDetection/imstkTetraToPointSetCD.cpp
    CollisionDetection/imstkUnidirectionalPlaneToCapsuleCD.cpp
    CollisionDetection/imstkUnidirectionalPlaneToSphereCD.cpp
    CollisionDetection/imstkLineMeshToSphereCD.cpp
    CollisionDetection/imstkLineMeshToCapsuleCD.cpp
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

if( ${PROJECT_NAME}_BUILD_BENCHMARK )
  add_subdirectory(Benchmarking)
endif()