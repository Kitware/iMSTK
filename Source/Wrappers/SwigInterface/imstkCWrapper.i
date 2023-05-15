/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. 
*/

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
#ifdef iMSTK_SYNCHRONOUS_LOGGING
#include "imstkLoggerSynchronous.h"
#else
#include "imstkLoggerG3.h"
#endif
#include "imstkModule.h"
#include "imstkModuleDriver.h"
#include "imstkColor.h"
#include "imstkEventObject.h"
#include "imstkTypes.h"
#include "imstkFactory.h"

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
#include "imstkAbstractCellMesh.h"
#include "imstkCellMesh.h"
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
#include "imstkCylinder.h"
#include "imstkGeometryUtilities.h"
#include "imstkSignedDistanceField.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"

/*
 * GeometryMappers
 */
#include "imstkGeometryMap.h"
#include "imstkPointwiseMap.h"
#include "imstkPointToTetMap.h"

/*
 * Filter
 */
#include "imstkGeometryAlgorithm.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkQuadricDecimate.h"
#include "imstkSelectEnclosedPoints.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSmoothen.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkSurfaceMeshTextureProject.h"
#include "imstkCleanMesh.h"

/* 
 * MeshIO 
 */
#include "imstkMeshIO.h"

/* 
 * DynamicalModel 
 */
#include "imstkPbdConstraint.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkVectorizedState.h"
#include "imstkAbstractDynamicalModel.h"
#include "imstkDynamicalModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkBackwardEuler.h"
#include "imstkPbdFemConstraint.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkSphBoundaryConditions.h"
#include "imstkInternalForceModelTypes.h"
#include "imstkFemDeformableBodyModel.h"
#include "imstkRigidBodyState2.h"
#include "imstkRigidBodyModel2.h"
#include "imstkSphState.h"
#include "imstkSphModel.h"

/* 
 * Rendering
 */
#include "imstkRenderMaterial.h"
#include "imstkTexture.h"

/*
 * Constraints
 */
#include "imstkPbdBody.h"
#include "imstkPbdConstraint.h"
#include "imstkRbdConstraint.h"
#include "imstkPbdDistanceConstraint.h"

/*
 * ComponentModel
 */
#include "imstkEntity.h"
#include "imstkComponent.h"

/* 
 * SceneEntities
 */
#include "imstkSceneObject.h"
#include "imstkCollidingObject.h"
#include "imstkDynamicObject.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCellRemoval.h"
#include "imstkTearable.h"
#include "imstkBurnable.h"
#include "imstkBurner.h"
#include "imstkVisualModel.h"
#include "imstkCamera.h"
#include "imstkLight.h"
#include "imstkDirectionalLight.h"
#include "imstkFeDeformableObject.h"
#include "imstkRigidObject2.h"
#include "imstkSphObject.h"

/*
 * CollisionDetection
 */
#include "imstkCDObjectFactory.h"
#include "imstkClosedSurfaceMeshToMeshCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCollisionUtils.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
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
#include "imstkRigidObjectController.h"
#include "imstkPbdObjectController.h"

/*
 * Needle
 */
#include "imstkArcNeedle.h"
#include "imstkNeedle.h"
#include "imstkNeedleInteraction.h"
#include "imstkNeedlePbdCH.h"
#include "imstkPuncturable.h"
#include "imstkPuncture.h"
#include "imstkStraightNeedle.h"
#include "imstkSurfaceInsertionConstraint.h"
#include "imstkThreadInsertionConstraint.h"

/*
 * Scene
 */
#include "imstkScene.h"
#include "imstkCollisionInteraction.h"
#include "imstkRigidObjectCollision.h"
#include "imstkPbdObjectCutting.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdRigidObjectCollision.h"
#include "imstkPbdRigidObjectGrasping.h"
#include "imstkSphObjectCollision.h"

/*
 * SimulationManager
 */
#include "imstkModule.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"

/*
 * ViewerCore
 */
#include "imstkViewer.h"

#ifdef iMSTK_USE_RENDERING_VTK
/*
 * ViewerVTK
 */
#include "imstkAbstractVTKViewer.h"
#include "imstkVTKViewer.h"
#endif

/*
 * Devices
 */
#include "imstkDeviceClient.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkMouseDeviceClient.h"

#ifdef iMSTK_USE_HAPLY
#include "imstkHaplyDeviceManager.h"
#include "imstkHaplyDeviceClient.h"
#endif

#ifdef iMSTK_USE_OpenHaptics
#include "imstkOpenHapticDeviceManager.h"
#include "imstkOpenHapticDeviceClient.h"
#endif

#ifdef iMSTK_USE_VRPN
#include "imstkVRPNDeviceManager.h"
#include "imstkVRPNDeviceClient.h"
#endif

#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"

%} /* end of module */

/*
 * stl
 */
%include <stdint.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_pair.i>

/*
 * https://github.com/swig/swig/pull/2480
 * Copied unordered_map for c# from the above pull request
 */

%include <std_unordered_map.i> 
namespace std
{
  %template(VectorInt) vector<int>;
  %template(VectorSizet) vector<std::size_t>;
  %template(VectorDouble) vector<double>;
  %template(VectorCollisionElement) vector<imstk::CollisionElement>;
  %template(VectorPbdBody) vector<imstk::PbdBody>;
  %template(VectorString) vector<std::string>;
  %template(IntPair) pair<int, int>;
  %template(UnorderedMapStringDouble) unordered_map<std::string, double>;
}

%include <std_except.i>
%include <exception.i>


%include "shared_ptr_instantiation.i"
%include "weak_ptr.i"
%include "ignored.i"
%include "modifiers.i"
%include "type_cast.i"
%include "std_function.i"
%include "callback.i"
%include "except.i"

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
%include "../../Geometry/Mesh/imstkImageData.h"
%include "../../Geometry/Mesh/imstkAbstractCellMesh.h"
%include "../../Geometry/Mesh/imstkCellMesh.h"
%template(CellMesh2) imstk::CellMesh<2>;
%template(CellMesh3) imstk::CellMesh<3>;
%template(CellMesh4) imstk::CellMesh<4>;
%template(CellMesh8) imstk::CellMesh<8>;
%include "../../Geometry/Mesh/imstkLineMesh.h"
%include "../../Geometry/Mesh/imstkSurfaceMesh.h"
%include "../../Geometry/Mesh/imstkVolumetricMesh.h"
%template(VolumetricMesh4) imstk::VolumetricMesh<4>;
%template(VolumetricMesh8) imstk::VolumetricMesh<8>;
%include "../../Geometry/Mesh/imstkTetrahedralMesh.h"
%include "../../Geometry/Mesh/imstkHexahedralMesh.h"
%include "../../Geometry/Implicit/imstkImplicitGeometry.h"
%include "../../Geometry/Implicit/imstkCompositeImplicitGeometry.h"
%include "../../Geometry/Analytic/imstkAnalyticalGeometry.h"
%include "../../Geometry/Analytic/imstkPlane.h"
%include "../../Geometry/Analytic/imstkSphere.h"
%include "../../Geometry/Analytic/imstkOrientedBox.h"
%include "../../Geometry/Analytic/imstkCapsule.h"
%include "../../Geometry/Analytic/imstkCylinder.h"
%include "../../Geometry/imstkGeometryUtilities.h"
%include "../../Geometry/Implicit/imstkSignedDistanceField.h"
%include "../../Geometry/Implicit/imstkImplicitFunctionFiniteDifferenceFunctor.h"

/*
 * GeometryMap
 */
%include "../../GeometryMappers/imstkGeometryMap.h"
%include "../../GeometryMappers/imstkPointwiseMap.h"
%include "../../GeometryMappers/imstkPointToTetMap.h"

/*
 * FilteringCore
 */
%include "../../FilteringCore/imstkGeometryAlgorithm.h"

/*
 * Filtering
 */
%include "../../Filtering/imstkImplicitGeometryToImageData.h"
%include "../../Filtering/imstkQuadricDecimate.h"
%include "../../Filtering/imstkSelectEnclosedPoints.h"
%include "../../Filtering/imstkSurfaceMeshFlyingEdges.h"
%include "../../Filtering/imstkSurfaceMeshSmoothen.h"
%include "../../Filtering/imstkSurfaceMeshSubdivide.h"
%include "../../Filtering/imstkSurfaceMeshTextureProject.h"
%include "../../Filtering/imstkCleanMesh.h"

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
%include "../../Constraint/PbdConstraints/imstkPbdBody.h"
%include "../../Constraint/PbdConstraints/imstkPbdConstraint.h"
%include "../../Constraint/PbdConstraints/imstkPbdConstraintContainer.h"
%include "../../Constraint/PbdConstraints/imstkPbdCollisionConstraint.h"
%include "../../Constraint/PbdConstraints/imstkPbdFemConstraint.h"
%include "../../Constraint/PbdConstraints/imstkPbdDistanceConstraint.h"
%include "../../Constraint/RigidBodyConstraints/imstkRbdConstraint.h"

/*
 * DynamicalModel
 */
%include "../../DynamicalModels/ObjectStates/imstkVectorizedState.h"
%include "../../DynamicalModels/ObjectModels/imstkAbstractDynamicalModel.h"
%include "../../DynamicalModels/ObjectModels/imstkDynamicalModel.h"
%include "../../DynamicalModels/ObjectModels/imstkPbdModelConfig.h"
%include "../../DynamicalModels/ObjectModels/imstkPbdModel.h"
%template(DynamicalModelFeDeformBodyState) imstk::DynamicalModel<imstk::FeDeformBodyState>;
%include "../../DynamicalModels/InternalForceModel/imstkInternalForceModelTypes.h"
%include "../../DynamicalModels/ObjectModels/imstkFemDeformableBodyModel.h"
%include "../../DynamicalModels/ObjectModels/imstkSphBoundaryConditions.h"
%include "../../DynamicalModels/TimeIntegrators/imstkTimeIntegrator.h"
%include "../../DynamicalModels/TimeIntegrators/imstkBackwardEuler.h"
%include "../../DynamicalModels/ObjectStates/imstkRigidBodyState2.h"
%template(DynamicalModelRigidBodyState2) imstk::DynamicalModel<imstk::RigidBodyState2>;
%include "../../DynamicalModels/ObjectModels/imstkRigidBodyModel2.h"
%include "../../DynamicalModels/ObjectStates/imstkSphState.h"
%template(DynamicalModelSphState) imstk::DynamicalModel<imstk::SphState>;
%include "../../DynamicalModels/ObjectModels/imstkSphModel.h"

/* 
 * Rendering 
 */
%include "../../Materials/imstkRenderMaterial.h";
%include "../../Materials/imstkTexture.h";

/*
 * ComponentModel
 */
%include "../../ComponentModel/imstkEntity.h"
%include "../../ComponentModel/imstkComponent.h"

/*
 * SceneEntities
 */
%include "../../SceneEntities/Components/imstkVisualModel.h";
%include "../../SceneEntities/Components/imstkTearable.h";
%include "../../SceneEntities/Components/imstkBurnable.h";
%include "../../SceneEntities/Components/imstkBurner.h";
%include "../../SceneEntities/Objects/imstkSceneObject.h";
%include "../../SceneEntities/Objects/imstkCollidingObject.h";
%include "../../SceneEntities/Objects/imstkDynamicObject.h";
%include "../../SceneEntities/Objects/imstkPbdConnectiveTissueConstraintGenerator.h";
%include "../../SceneEntities/Objects/imstkPbdObject.h";
%include "../../SceneEntities/Objects/imstkPbdObjectCellRemoval.h";
%include "../../SceneEntities/Objects/imstkFeDeformableObject.h";
%include "../../SceneEntities/Objects/imstkRigidObject2.h";
%include "../../SceneEntities/Objects/imstkSphObject.h";
%include "../../SceneEntities/Camera/imstkCamera.h";
%include "../../SceneEntities/Lights/imstkLight.h";
%include "../../SceneEntities/Lights/imstkDirectionalLight.h";

/*
 * CollisionDetection
 */
%include "../../CollisionDetection/imstkCDObjectFactory.h"

%extend imstk::CDObjectFactory {
	static std::vector<std::string> getNames() {
		return imstk::CDObjectFactory::getNames();
	}
}

%include "../../CollisionDetection/imstkCollisionData.h"
%include "../../CollisionDetection/imstkCollisionDetectionAlgorithm.h"
%include "../../CollisionDetection/imstkCollisionUtils.h"

%include "../../CollisionDetection/CollisionDetection/imstkBidirectionalPlaneToSphereCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkClosedSurfaceMeshToMeshCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkImplicitGeometryToPointSetCCD.h"
%include "../../CollisionDetection/CollisionDetection/imstkImplicitGeometryToPointSetCD.h"
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
%include "../../Controllers/imstkRigidObjectController.h"
%include "../../Controllers/imstkPbdObjectController.h"

/* 
 * Scene
 */
%include "../../Scene/imstkScene.h";
%include "../../Scene/imstkCollisionInteraction.h"
%include "../../Scene/imstkRigidObjectCollision.h"
%include "../../Scene/imstkPbdObjectCutting.h"
%include "../../Scene/imstkPbdObjectGrasping.h"
%include "../../Scene/imstkPbdObjectCollision.h"
%include "../../Scene/imstkPbdRigidObjectCollision.h"
%include "../../Scene/imstkPbdRigidObjectGrasping.h"
%include "../../Scene/imstkSphObjectCollision.h"

/*
 * Needle
 */
%include "../../Needle/imstkPuncture.h"
%include "../../Needle/imstkNeedle.h"
%include "../../Needle/imstkStraightNeedle.h"
%include "../../Needle/imstkArcNeedle.h"
%include "../../Needle/imstkPuncturable.h"
%include "../../Needle/imstkNeedlePbdCH.h"
%include "../../Needle/imstkNeedleInteraction.h"

/*
 * ViewerCore
 */
%include "../../ViewerCore/imstkViewer.h";

#ifdef iMSTK_USE_RENDERING_VTK
/*
 * ViewerVTK
 */
%include "../../ViewerVTK/imstkAbstractVTKViewer.h";
%include "../../ViewerVTK/imstkVTKViewer.h";
#endif

/*
 * SimulationManager
 */
%include "../../SimulationManager/imstkSceneManager.h"
%include "../../SimulationManager/imstkSimulationManager.h"
%include "../../SimulationManager/imstkMouseSceneControl.h"
%include "../../SimulationManager/imstkKeyboardSceneControl.h"

/*
 * Devices
 */
%include "../../Devices/imstkDeviceClient.h"
%include "../../Devices/imstkKeyboardDeviceClient.h"
%include "../../Devices/imstkMouseDeviceClient.h"
%include "../../Devices/imstkDeviceManager.h"
%include "../../Devices/imstkDeviceManagerFactory.h"

/*
 * The Superclass static functions don't seem to get exposed, 
 * this adds a "local" static function that just invokes the builtin
 * contains function
 */
%extend imstk::DeviceManagerFactory {
	static bool contains(const std::string& val) {
		return imstk::DeviceManagerFactory::contains(val);
	}
}

#ifdef iMSTK_USE_HAPLY
	%include "../../Devices/imstkHaplyDeviceManager.h"
	%include "../../Devices/imstkHaplyDeviceClient.h"
#endif

#ifdef iMSTK_USE_OpenHaptics
	#define HDCALLBACK
	%include "../../Devices/imstkOpenHapticDeviceManager.h"
	%include "../../Devices/imstkOpenHapticDeviceClient.h"
#endif

#ifdef iMSTK_USE_VRPN
	// The static calls in DeviceClient are getting ignored anyway define these
	// Rather than dealing with the correct includes for VRPN
	#define VRPN_CALLBACK
	#define _vrpn_TRACKERCB void*
	#define _vrpn_TRACKERVELCB void*
	#define _vrpn_ANALOGCB void*
	#define _vrpn_BUTTONCB void*

	// Swig things that VRPNDeviceManager is abstract, that will
	// mean no constructors are created
	%feature("notabstract") imstk::VRPNDeviceManager;

	%include "../../Devices/imstkVRPNDeviceManager.h"
	%include "../../Devices/imstkVRPNDeviceClient.h"
#endif
