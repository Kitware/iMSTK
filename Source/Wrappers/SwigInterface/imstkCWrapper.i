%module(directors="1") Utils
#pragma SWIG nowarn=302,314,317,401,476,501,503,505,516,844,
%{
/* Common */
#include "imstkMacros.h"
#include "imstkMath.h"
#include "imstkAbstractDataArray.h"
#include "imstkDataArray.h"
#include "imstkVecDataArray.h"
#include "imstkLogger.h"
#include "imstkModule.h"
#include "imstkModuleDriver.h"
#include "imstkColor.h"
#include "imstkEventObject.h"
#include "imstkTypes.h"

/*
 * DataStructures
 */
#include "imstkNeighborSearch.h"

/* 
 * Geometry 
 */
#include "imstkGeometry.h"
#include "imstkGeometryUtilities.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkImageData.h"
#include "imstkVolumetricMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkImplicitGeometry.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCompositeImplicitGeometry.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkOrientedBox.h"
#include "imstkCapsule.h"
#include "imstkGeometryUtilities.h"
#include "imstkSignedDistanceField.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"

/*
 * GeometryMappers
 */
#include "imstkGeometryMap.h"
#include "imstkOneToOneMap.h"
#include "imstkTetraTriangleMap.h"

/*
 * Filter
 */
#include "imstkGeometryAlgorithm.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSelectEnclosedPoints.h"

/* 
 * MeshIO 
 */
#include "imstkMeshIO.h"

/* 
 * DynamicalModel 
 */
#include "imstkVectorizedState.h"
#include "imstkPbdState.h"
#include "imstkAbstractDynamicalModel.h"
#include "imstkDynamicalModel.h"
#include "imstkPbdModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkBackwardEuler.h"
#include "imstkPbdFEMConstraint.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkSPHBoundaryConditions.h"
#include "imstkInternalForceModelTypes.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkRigidBodyState2.h"
#include "imstkRigidBodyModel2.h"
#include "imstkSPHState.h"
#include "imstkSPHModel.h"

/* 
 * Rendering
 */
#include "imstkRenderMaterial.h"
#include "imstkTexture.h"

/*
 * Constraints
 */
#include "imstkPbdConstraint.h"
#include "imstkRbdConstraint.h"

/* 
 * SceneEntities
 */
#include "imstkSceneEntity.h"
#include "imstkSceneObject.h"
#include "imstkCollidingObject.h"
#include "imstkDynamicObject.h"
#include "imstkPbdObject.h"
#include "imstkVisualModel.h"
#include "imstkCamera.h"
#include "imstkLight.h"
#include "imstkDirectionalLight.h"
#include "imstkFeDeformableObject.h"
#include "imstkRigidObject2.h"
#include "imstkSPHObject.h"

/*
 * CollisionDetection
 */
#include "imstkCollisionData.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCollisionUtils.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkSphereToCylinderCD.h"
#include "imstkSphereToSphereCD.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkTetraToLineMeshCD.h"
#include "imstkTetraToPointSetCD.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"


/*
 * CollisionHandling
 */
#include "imstkCollisionHandling.h"
#include "imstkRigidBodyCH.h"
/*
 * Controller
 */
#include "imstkDeviceControl.h"
#include "imstkMouseControl.h"
#include "imstkKeyboardControl.h"
#include "imstkTrackingDeviceControl.h"
#include "imstkSceneObjectController.h"

/*
 * Scene
 */
#include "imstkScene.h"
#include "imstkCollisionGraph.h"
#include "imstkObjectInteractionPair.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkCollisionPair.h"
#include "imstkRigidObjectCollision.h"
#include "imstkInteractionPair.h"
#include "imstkObjectInteractionPair.h"
#include "imstkPbdObjectCuttingPair.h"
#include "imstkPbdObjectCollision.h"
#include "imstkSphObjectCollision.h"

/*
 * SimulationManager
 */
#include "imstkModule.h"
#include "imstkViewer.h"
#include "imstkAbstractVTKViewer.h"
#include "imstkVTKViewer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"

/*
 * Devices
 */
#include "imstkDeviceClient.h"
#include "imstkKeyboardDeviceClient.h"
#ifdef iMSTK_USE_OpenHaptics
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#endif

%} /* end of module */

/*
 * stl
 */
%include <stdint.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_pair.i>
namespace std
{
  %template(VectorInt) vector<int>; 
  %template(VectorSizet) vector<std::size_t>;
  %template(VectorCollisionElement) vector<imstk::CollisionElement>;
  %template(PbdTypeStiffnessPair) pair<imstk::PbdConstraint::Type, double>;
  %template(VectorPbdTypeStiffnessPair) vector<std::pair<imstk::PbdConstraint::Type, double>>;
}

%include "shared_ptr_instantiation.i"
%include "weak_ptr.i"
%include "ignored.i"
%include "modifiers.i"
%include "type_cast.i"
%include "std_function.i"
%include "callback.i"

/* rename these operators to "compute" due to lack of operator overloading */
%rename(compute) imstk::ImplicitFunctionGradient::operator();
%rename(compute) imstk::ImplicitFunctionCentralGradient::operator();

/*
 * * * * * * * * * * * * * * * *
 * list C/C++ declarations
 * * * * * * * * * * * * * * * * *
 */
/*
 * Common
 */
%include "common.i"

/*
 * DataStructures
 */
%include "../../DataStructures/imstkNeighborSearch.h"

/*
 * Geometry
 */
%include "../../Geometry/imstkGeometry.h";
%include "../../Geometry/Mesh/imstkPointSet.h"
%include "../../Geometry/Mesh/imstkSurfaceMesh.h"
%include "../../Geometry/Mesh/imstkLineMesh.h"
%include "../../Geometry/Mesh/imstkImageData.h"
%include "../../Geometry/Mesh/imstkVolumetricMesh.h"
%include "../../Geometry/Mesh/imstkTetrahedralMesh.h"
%include "../../Geometry/Mesh/imstkHexahedralMesh.h"
%include "../../Geometry/Implicit/imstkImplicitGeometry.h"
%include "../../Geometry/Implicit/imstkCompositeImplicitGeometry.h"
%include "../../Geometry/Analytic/imstkAnalyticalGeometry.h"
%include "../../Geometry/Analytic/imstkPlane.h"
%include "../../Geometry/Analytic/imstkSphere.h"
%include "../../Geometry/Analytic/imstkOrientedBox.h"
%include "../../Geometry/Analytic/imstkCapsule.h"
%include "../../Geometry/imstkGeometryUtilities.h"
%include "../../Geometry/Implicit/imstkSignedDistanceField.h"
%include "../../Geometry/Implicit/imstkImplicitFunctionFiniteDifferenceFunctor.h"

/*
 * GeometryMap
 */
%include "../../GeometryMappers/imstkGeometryMap.h"
%include "../../GeometryMappers/imstkOneToOneMap.h"
%include "../../GeometryMappers/imstkTetraTriangleMap.h"

/*
 * Filtering
 */
%include "../../FilteringCore/imstkGeometryAlgorithm.h"
%include "../../Filtering/imstkSurfaceMeshSubdivide.h"
%include "../../Filtering/imstkImplicitGeometryToImageData.h"
%include "../../Filtering/imstkSurfaceMeshFlyingEdges.h"
%include "../../Filtering/imstkSelectEnclosedPoints.h"

/*
 * MeshIO
 */
%include "../../MeshIO/imstkMeshIO.h";
%template(readImageData) imstk::MeshIO::read<imstk::ImageData>;
%template(readPointSet) imstk::MeshIO::read<imstk::PointSet>;
%template(readSurfaceMesh) imstk::MeshIO::read<imstk::SurfaceMesh>;
%template(readTetrahedralMesh) imstk::MeshIO::read<imstk::TetrahedralMesh>;

/*
 * Constraint
 */
%include "../../Constraint/PbdConstraints/imstkPbdConstraint.h"
%include "../../Constraint/PbdConstraints/imstkPbdCollisionConstraint.h"
%include "../../Constraint/PbdConstraints/imstkPbdFEMConstraint.h"
%include "../../Constraint/RigidBodyConstraints/imstkRbdConstraint.h"

/*
 * DynamicalModel
 */
%include "../../DynamicalModels/ObjectStates/imstkVectorizedState.h"
%include "../../DynamicalModels/ObjectStates/imstkPbdState.h"
%include "../../DynamicalModels/ObjectModels/imstkAbstractDynamicalModel.h"
%include "../../DynamicalModels/ObjectModels/imstkDynamicalModel.h"
/* Instantiation of base class should be put before the derived class */
%template(DynamicalModelPbdState) imstk::DynamicalModel<imstk::PbdState>;
%include "../../DynamicalModels/ObjectModels/imstkPbdModel.h"
%template(DynamicalModelFeDeformBodyState) imstk::DynamicalModel<imstk::FeDeformBodyState>;
%include "../../DynamicalModels/InternalForceModel/imstkInternalForceModelTypes.h"
%include "../../DynamicalModels/ObjectModels/imstkFEMDeformableBodyModel.h"
%include "../../DynamicalModels/ObjectModels/imstkSPHBoundaryConditions.h"
/* %include "../../DynamicalModels/ObjectModels/imstkSPHHemorrhage.h" */
%include "../../DynamicalModels/TimeIntegrators/imstkTimeIntegrator.h"
%include "../../DynamicalModels/TimeIntegrators/imstkBackwardEuler.h"
%include "../../DynamicalModels/ObjectStates/imstkRigidBodyState2.h"
%template(DynamicalModelRigidBodyState2) imstk::DynamicalModel<imstk::RigidBodyState2>;
%include "../../DynamicalModels/ObjectModels/imstkRigidBodyModel2.h"
%include "../../DynamicalModels/ObjectStates/imstkSPHState.h"
%template(DynamicalModelSPHState) imstk::DynamicalModel<imstk::SPHState>;
%include "../../DynamicalModels/ObjectModels/imstkSPHModel.h"

/* 
 * Rendering 
 */
%include "../../Rendering/Materials/imstkRenderMaterial.h";
%include "../../Rendering/Materials/imstkTexture.h";

/*
 * SceneEntities
 */
%include "../../SceneEntities/imstkSceneEntity.h"
%include "../../SceneEntities/Objects/imstkSceneObject.h";
%include "../../SceneEntities/Objects/imstkCollidingObject.h";
%include "../../SceneEntities/Objects/imstkDynamicObject.h";
%include "../../SceneEntities/Objects/imstkPbdObject.h";
%include "../../SceneEntities/Objects/imstkVisualModel.h";
%include "../../SceneEntities/Objects/imstkFeDeformableObject.h";
%include "../../SceneEntities/Objects/imstkRigidObject2.h";
%include "../../SceneEntities/Objects/imstkSPHObject.h";
%include "../../SceneEntities/Camera/imstkCamera.h";
%include "../../SceneEntities/Lights/imstkLight.h";
%include "../../SceneEntities/Lights/imstkDirectionalLight.h";

/*
 * CollisionDetection
 */
/* %include "../../CollisionDetection/CollisionDetection/imstkCollisionDetection.h"; */
%include "../../CollisionDetection/CollisionData/imstkCollisionData.h";
%include "../../CollisionDetection/CollisionDetection/imstkCollisionDetectionAlgorithm.h"
%include "../../CollisionDetection/CollisionDetection/imstkBidirectionalPlaneToSphereCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkCollisionUtils.h"
%include "../../CollisionDetection/CollisionDetection/imstkImplicitGeometryToPointSetCCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkImplicitGeometryToPointSetCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkMeshToMeshBruteForceCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkPointSetToCapsuleCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkPointSetToOrientedBoxCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkPointSetToPlaneCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkPointSetToSphereCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkSphereToCylinderCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkSphereToSphereCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkSurfaceMeshToCapsuleCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkSurfaceMeshToSphereCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkSurfaceMeshToSurfaceMeshCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkTetraToLineMeshCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkTetraToPointSetCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkUnidirectionalPlaneToSphereCD.h"



/*
 * CollisionHandling
 */ 
%include "../../CollisionHandling/imstkCollisionHandling.h";
%include "../../CollisionHandling/imstkRigidBodyCH.h";

/* 
 * Controllers
 */
%include "../../Controllers/imstkDeviceControl.h"
%include "../../Controllers/imstkMouseControl.h"
%include "../../Controllers/imstkKeyboardControl.h"
%include "../../Controllers/imstkTrackingDeviceControl.h"
%include "../../Controllers/imstkSceneObjectController.h"

/* 
 * Scene
 */
%include "../../Scene/imstkScene.h";
%include "../../Scene/imstkCollisionGraph.h";
%include "../../Scene/imstkInteractionPair.h"
%include "../../Scene/imstkObjectInteractionPair.h";
%include "../../Scene/imstkObjectInteractionFactory.h";
%include "../../Scene/imstkCollisionPair.h";
%include "../../Scene/imstkRigidObjectCollision.h";
%include "../../Scene/imstkPbdObjectCuttingPair.h"
%include "../../Scene/imstkPbdObjectCollision.h"
%include "../../Scene/imstkSphObjectCollision.h"

/*
 * SimulationManager
 */
%include "../../SimulationManager/imstkViewer.h";
%include "../../SimulationManager/VTKRenderer/imstkAbstractVTKViewer.h";
%include "../../SimulationManager/VTKRenderer/imstkVTKViewer.h";
%include "../../SimulationManager/VTKRenderer/imstkVTKTextStatusManager.h";
%include "../../SimulationManager/imstkSceneManager.h"
%include "../../SimulationManager/imstkSimulationManager.h"
%include "../../SimulationManager/imstkMouseSceneControl.h"
%include "../../SimulationManager/imstkKeyboardSceneControl.h"

/*
 * Devices
 */
%include "../../Devices/imstkDeviceClient.h"
%include "../../Devices/imstkKeyboardDeviceClient.h"
#ifdef iMSTK_USE_OpenHaptics
%include "../../Devices/imstkHapticDeviceManager.h"
%include "../../Devices/imstkHapticDeviceClient.h"
#endif