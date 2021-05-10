%module(directors="1") modIMSTK
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
#include "imstkCube.h"
#include "imstkGeometryUtilities.h"

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
#include "imstkPbdConstraint.h"
#include "imstkTimeIntegrator.h"
#include "imstkBackwardEuler.h"
#include "imstkPbdFEMConstraint.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkInternalForceModelTypes.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkRigidBodyState2.h"
#include "imstkRigidBodyModel2.h"

/* 
 * Rendering
 */
#include "imstkRenderMaterial.h"
#include "imstkTexture.h"

/*
 * Constraints
 */
#include "imstkRbdConstraint.h"

/* 
 * SceneEntities
 */
#include "imstkSceneObject.h"
#include "imstkCollidingObject.h"
#include "imstkDynamicObject.h"
#include "imstkPbdObject.h"
#include "imstkVisualModel.h"
#include "imstkCamera.h"
#include "imstkLight.h"
#include "imstkFeDeformableObject.h"
#include "imstkRigidObject2.h"

/*
 * CollisionDetection
 */
#include "imstkCollisionDetection.h"

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

/*
 * Scene
 */
#include "imstkScene.h"
#include "imstkCollisionGraph.h"
#include "imstkObjectInteractionPair.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkCollisionPair.h"
#include "imstkRigidObjectCollidingCollisionPair.h"
#include "imstkInteractionPair.h"
#include "imstkObjectInteractionPair.h"

/*
 * SimulationManager
 */
#include "imstkModule.h"
#include "imstkViewer.h"
#include "imstkAbstractVTKViewer.h"
#include "imstkVTKViewer.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"

/*
 * Devices
 */
#include "imstkDeviceClient.h"
#include "imstkKeyboardDeviceClient.h"

%} /* end of module */


/*
 * stl
 */
%include <std_vector.i>
namespace std {
%template(VectorInt) vector<int>; 
%template(VectorSizet) vector<std::size_t>; 
}
%include <std_string.i>

%include "shared_ptr_instantiation.i"

// Instantiation of std::weak_ptr
%import "weak_ptr.i"
%template(SceneManagerWeakPtr) std::weak_ptr<imstk::SceneManager>;
%template(ModuleDriverWeakPtr) std::weak_ptr<imstk::ModuleDriver>;
%template(SimulationManagerWeakPtr) std::weak_ptr<imstk::SimulationManager>;

%include "cpointer.i"
%include "arrays_csharp.i"
%apply unsigned char INPUT[] {const unsigned char* val}
%apply int INPUT[] {const int* val}
%apply float INPUT[] {const float * val}
%apply double INPUT[] {const double* val}
%apply unsigned char OUTPUT[] {unsigned char* val}
%apply int OUTPUT[] {int* val}
%apply float OUTPUT[] {float * val}
%apply double OUTPUT[] {double* val}
%include "ignored.i"
%include "ignored.i"
%include "type_cast.i"
%include "std_function.i"
%std_function(ReceiverFunc, void, imstk::KeyEvent*)

%callback("%s_cb");
/* std::string imstk::KeyboardDeviceClient::keyPress(); */
std::string KeyboardDeviceClient_getKeyPress();
std::string Module_getPostUpdate();
std::string Module_getPreUpdate();
%nocallback;


/* namespace imstk { */
/* %inline %{ */
%{
std::string KeyboardDeviceClient_getKeyPress() {
  return imstk::KeyboardDeviceClient::keyPress();
}

std::string Module_getPostUpdate() {
  return imstk::Module::postUpdate();
}
std::string Module_getPreUpdate() {
  return imstk::Module::preUpdate();
}
%}
/* } */

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
 * Geometry
 */
%include "../Geometry/imstkGeometry.h";
%include "../Geometry/Mesh/imstkPointSet.h"
%include "../Geometry/Mesh/imstkSurfaceMesh.h"
%include "../Geometry/Mesh/imstkLineMesh.h"
%include "../Geometry/Mesh/imstkImageData.h"
%include "../Geometry/Mesh/imstkVolumetricMesh.h"
%include "../Geometry/Mesh/imstkTetrahedralMesh.h"
%include "../Geometry/Mesh/imstkHexahedralMesh.h"
%include "../Geometry/Implicit/imstkImplicitGeometry.h"
%include "../Geometry/Implicit/imstkCompositeImplicitGeometry.h"
%include "../Geometry/Analytic/imstkAnalyticalGeometry.h"
%include "../Geometry/Analytic/imstkPlane.h"
%include "../Geometry/Analytic/imstkSphere.h"
%include "../Geometry/Analytic/imstkCube.h"
%include "../Geometry/imstkGeometryUtilities.h"

/*
 * GeometryMap
 */
%include "../GeometryMappers/imstkGeometryMap.h"
%include "../GeometryMappers/imstkOneToOneMap.h"
%include "../GeometryMappers/imstkTetraTriangleMap.h"

/*
 * Filtering
 */
%include "../Filtering/imstkGeometryAlgorithm.h"
%include "../Filtering/imstkSurfaceMeshSubdivide.h"
%include "../Filtering/imstkImplicitGeometryToImageData.h"
%include "../Filtering/imstkSurfaceMeshFlyingEdges.h"

/*
 * MeshIO
 */
%include "../MeshIO/imstkMeshIO.h";
%template(readImageData) imstk::MeshIO::read<imstk::ImageData>;
%template(readSurfaceMesh) imstk::MeshIO::read<imstk::SurfaceMesh>;
%template(readTetrahedralMesh) imstk::MeshIO::read<imstk::TetrahedralMesh>;

/*
 * Constraint
 */
%include "../Constraint/PbdConstraints/imstkPbdConstraint.h"
%include "../Constraint/PbdConstraints/imstkPbdCollisionConstraint.h"
%include "../Constraint/PbdConstraints/imstkPbdFEMConstraint.h"

/*
 * DynamicalModel
 */
%include "../DynamicalModels/ObjectStates/imstkVectorizedState.h"
%include "../DynamicalModels/ObjectStates/imstkPbdState.h"
%include "../DynamicalModels/ObjectModels/imstkAbstractDynamicalModel.h"
%include "../DynamicalModels/ObjectModels/imstkDynamicalModel.h"
/* Instantiation of base class should be put before the derived class */
%template(DynamicalModelPbdState) imstk::DynamicalModel<imstk::PbdState>;
%include "../DynamicalModels/ObjectModels/imstkPbdModel.h"
%template(DynamicalModelFeDeformBodyState) imstk::DynamicalModel<imstk::FeDeformBodyState>;
%include "../DynamicalModels/InternalForceModel/imstkInternalForceModelTypes.h"
%include "../DynamicalModels/ObjectModels/imstkFEMDeformableBodyModel.h"
%include "../DynamicalModels/TimeIntegrators/imstkTimeIntegrator.h"
%include "../DynamicalModels/TimeIntegrators/imstkBackwardEuler.h"
%include "../DynamicalModels/ObjectStates/imstkRigidBodyState2.h"
%template(DynamicalModelRigidBodyState2) imstk::DynamicalModel<imstk::RigidBodyState2>;
%include "../DynamicalModels/ObjectModels/imstkRigidBodyModel2.h"

/* 
 * Rendering 
 */
%include "../Rendering/Materials/imstkRenderMaterial.h";
%include "../Rendering/Materials/imstkTexture.h";

/*
 * Constraints
 */
%include "../Constraint/RigidBodyConstraints/imstkRbdConstraint.h"
/*
 * SceneEntities
 */
%include "../SceneEntities/Objects/imstkSceneObject.h";
%include "../SceneEntities/Objects/imstkCollidingObject.h";
%include "../SceneEntities/Objects/imstkDynamicObject.h";
%include "../SceneEntities/Objects/imstkPbdObject.h";
%include "../SceneEntities/Objects/imstkVisualModel.h";
%include "../SceneEntities/Objects/imstkFeDeformableObject.h";
%include "../SceneEntities/Objects/imstkRigidObject2.h";
%include "../SceneEntities/Camera/imstkCamera.h";
%include "../SceneEntities/Lights/imstkLight.h";

/*
 * CollisionDetection
 */
%include "../CollisionDetection/CollisionDetection/imstkCollisionDetection.h";

/*
 * CollisionHandling
 */ 
%include "../CollisionHandling/imstkCollisionHandling.h";
%include "../CollisionHandling/imstkRigidBodyCH.h";

/* 
 * Controllers
 */
%include "../Controllers/imstkDeviceControl.h"
%include "../Controllers/imstkMouseControl.h"
%include "../Controllers/imstkKeyboardControl.h"

/* 
 * Scene
 */
%include "../Scene/imstkScene.h";
%include "../Scene/imstkCollisionGraph.h";
%include "../Scene/imstkObjectInteractionPair.h";
%include "../Scene/imstkObjectInteractionFactory.h";
%include "../Scene/imstkCollisionPair.h";
%include "../Scene/imstkRigidObjectCollidingCollisionPair.h";
%include "../Scene/imstkInteractionPair.h"
%include "../Scene/imstkObjectInteractionPair.h"

/*
 * SimulationManager
 */
%include "../SimulationManager/imstkViewer.h";
%include "../SimulationManager/VTKRenderer/imstkAbstractVTKViewer.h";
%include "../SimulationManager/VTKRenderer/imstkVTKViewer.h";
%include "../SimulationManager/imstkSceneManager.h"
%include "../SimulationManager/imstkSimulationManager.h"
%include "../SimulationManager/imstkMouseSceneControl.h"
%include "../SimulationManager/imstkKeyboardSceneControl.h"

/*
 * Devices
 */
%include "../Devices/imstkDeviceClient.h"
%include "../Devices/imstkKeyboardDeviceClient.h"

