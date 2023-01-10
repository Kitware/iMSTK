/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. 
*/

/*
 * Instantiation of shared_ptr
 */
%include <std_shared_ptr.i>

/*
 * Common
 */
%shared_ptr(imstk::AbstractDataArray)
%shared_ptr(imstk::DataArray<int>)
%shared_ptr(imstk::DataArray<float>)
%shared_ptr(imstk::DataArray<double>)
%shared_ptr(imstk::DataArray<unsigned char>)
%shared_ptr(imstk::VecDataArray<int, 2>)
%shared_ptr(imstk::VecDataArray<int, 3>)
%shared_ptr(imstk::VecDataArray<int, 4>)
%shared_ptr(imstk::VecDataArray<float, 2>)
%shared_ptr(imstk::VecDataArray<double, 2>)
%shared_ptr(imstk::VecDataArray<double, 3>)
%shared_ptr(imstk::VecDataArray<unsigned char, 3>)
%shared_ptr(imstk::ModuleDriver)
%shared_ptr(imstk::Event)
%shared_ptr(imstk::EventObject)
%shared_ptr(imstk::ButtonEvent)
%shared_ptr(imstk::KeyEvent)

/* 
 * Geometry 
 */
%shared_ptr(imstk::Geometry)
%shared_ptr(imstk::AbstractCellMesh)
%shared_ptr(imstk::PointSet)
%shared_ptr(imstk::ImageData)
%shared_ptr(imstk::CellMesh<2>)
%shared_ptr(imstk::CellMesh<3>)
%shared_ptr(imstk::CellMesh<4>)
%shared_ptr(imstk::CellMesh<8>)
%shared_ptr(imstk::LineMesh)
%shared_ptr(imstk::SurfaceMesh)
%shared_ptr(imstk::VolumetricMesh<4>)
%shared_ptr(imstk::VolumetricMesh<8>)
%shared_ptr(imstk::TetrahedralMesh)
%shared_ptr(imstk::HexahedralMesh)
%shared_ptr(imstk::ImplicitGeometry)
%shared_ptr(imstk::AnalyticalGeometry)
%shared_ptr(imstk::CompositeImplicitGeometry)
%shared_ptr(imstk::Plane)
%shared_ptr(imstk::Sphere)
%shared_ptr(imstk::OrientedBox)
%shared_ptr(imstk::Capsule)
%shared_ptr(imstk::Cylinder)
%shared_ptr(imstk::SignedDistanceField)

/* 
 * GeometryMap
 */
%shared_ptr(imstk::GeometryMap)
%shared_ptr(imstk::PointwiseMap)
%shared_ptr(imstk::PointToTetMap)

/*
 * FilteringCore
 */
%shared_ptr(imstk::GeometryAlgorithm)

/*
 * Filtering
 */
%shared_ptr(imstk::ImplicitGeometryToImageData)
%shared_ptr(imstk::QuadricDecimate)
%shared_ptr(imstk::SelectEnclosedPoints)
%shared_ptr(imstk::SurfaceMeshFlyingEdges)
%shared_ptr(imstk::SurfaceMeshSmoothen)
%shared_ptr(imstk::SurfaceMeshSubdivide)
%shared_ptr(imstk::SurfaceMeshTextureProject)

/* 
 * DynamicalModel 
 */
%shared_ptr(imstk::PbdObject)
%shared_ptr(imstk::PbdModelConfig)
%shared_ptr(imstk::PbdCollisionConstraintConfig)
%shared_ptr(imstk::PbdFemConstraintConfig)
%shared_ptr(imstk::PbdSolver)
%shared_ptr(imstk::FeDeformBodyState)
%shared_ptr(imstk::FemModelConfig)
%shared_ptr(imstk::FemDeformableBodyModel)
%shared_ptr(imstk::AbstractDynamicalModel)
%shared_ptr(imstk::DynamicalModel<imstk::FeDeformBodyState>)
%shared_ptr(imstk::DynamicalModel<imstk::SphState>)
%shared_ptr(imstk::SphState)
%shared_ptr(imstk::RigidBodyState2)
%shared_ptr(imstk::DynamicalModel<imstk::RigidBodyState2>)
%shared_ptr(imstk::PbdModel)
%shared_ptr(imstk::RigidBodyModel2Config)
%shared_ptr(imstk::RigidBodyModel2)
%shared_ptr(imstk::SphModelConfig)
%shared_ptr(imstk::SphModel)
%shared_ptr(imstk::TimeIntegrator)
%shared_ptr(imstk::BackwardEuler)

/* 
 * Materials
 */
%shared_ptr(imstk::RenderMaterial)
%shared_ptr(imstk::Texture)

/*
 * Constraint
 */
%shared_ptr(imstk::PbdBody)
%shared_ptr(imstk::PbdState)
%shared_ptr(imstk::RigidBody)
%shared_ptr(imstk::PbdConstraint)
%shared_ptr(imstk::PbdFemConstraint)

/*
 * ComponentModel
 */
%shared_ptr(imstk::Entity)
%shared_ptr(imstk::Component)

/* 
 * SceneEntities
 */
%shared_ptr(imstk::SceneObject)
%shared_ptr(imstk::CollidingObject)
%shared_ptr(imstk::DynamicObject)
%shared_ptr(imstk::PbdConnectiveTissueConstraintGenerator)
%shared_ptr(imstk::PbdObject)
%shared_ptr(imstk::FeDeformableObject)
%shared_ptr(imstk::SphObject)
%shared_ptr(imstk::RigidObject2)
%shared_ptr(imstk::VisualModel)
%shared_ptr(imstk::Camera)
%shared_ptr(imstk::Light)
%shared_ptr(imstk::DirectionalLight)
%shared_ptr(imstk::PointLight)
%shared_ptr(imstk::Tearable)
%shared_ptr(imstk::Burnable)
%shared_ptr(imstk::Burner)
%shared_ptr(imstk::SpotLight)

/*
 * CollisionDetection
 */
%shared_ptr(imstk::CollisionDetectionAlgorithm)

%shared_ptr(imstk::ClosedSurfaceMeshToMeshCD)
%shared_ptr(imstk::BidirectionalPlaneToSphereCD)
%shared_ptr(imstk::ImplicitGeometryToPointSetCCD)
%shared_ptr(imstk::ImplicitGeometryToPointSetCD)
%shared_ptr(imstk::PointSetToCapsuleCD)
%shared_ptr(imstk::PointSetToOrientedBoxCD)
%shared_ptr(imstk::PointSetToSphereCD)
%shared_ptr(imstk::PointSetToPlaneCD)
%shared_ptr(imstk::SphereToCylinderCD)
%shared_ptr(imstk::SphereToSphereCD)
%shared_ptr(imstk::SurfaceMeshToCapsuleCD)
%shared_ptr(imstk::SurfaceMeshToSphereCD)
%shared_ptr(imstk::SurfaceMeshToSurfaceMeshCD)
%shared_ptr(imstk::TetraToLineMeshCD)
%shared_ptr(imstk::TetraToPointSetCD)
%shared_ptr(imstk::UnidirectionalPlaneToSphereCD)

/*
 * CollisionHandling
 */
%shared_ptr(imstk::CollisionPair)
%shared_ptr(imstk::CollisionHandling)
%shared_ptr(imstk::RigidObjectCollidingCollisionPair)
%shared_ptr(imstk::RigidBodyCH)

/*
 * Controller
 */
%shared_ptr(imstk::DeviceControl)
%shared_ptr(imstk::MouseControl)
%shared_ptr(imstk::KeyboardControl)
%shared_ptr(imstk::TrackingDeviceControl)
%shared_ptr(imstk::SceneObjectController)
%shared_ptr(imstk::RigidObjectController)
%shared_ptr(imstk::PbdObjectController)

/*
 * Needle
 */
%shared_ptr(imstk::Puncture)
%shared_ptr(imstk::Needle)
%shared_ptr(imstk::StraightNeedle)
%shared_ptr(imstk::ArcNeedle)
%shared_ptr(imstk::Puncturable)

/*
 * Scene
 */
%shared_ptr(imstk::SceneConfig)
%shared_ptr(imstk::Scene)
%shared_ptr(imstk::CollisionInteraction)
%shared_ptr(imstk::PbdObjectCellRemoval)
%shared_ptr(imstk::RigidObjectCollision)
%shared_ptr(imstk::PbdObjectCutting)
%shared_ptr(imstk::PbdObjectGrasping)
%shared_ptr(imstk::PbdObjectCollision)
%shared_ptr(imstk::PbdRigidObjectCollision)
%shared_ptr(imstk::PbdRigidObjectGrasping)
%shared_ptr(imstk::SphObjectCollision)

/*
 * SimulationManager
 */
%shared_ptr(imstk::Module)
%shared_ptr(imstk::Viewer)
%shared_ptr(imstk::SceneManager)
%shared_ptr(imstk::SimulationManager)
%shared_ptr(imstk::MouseSceneControl)
%shared_ptr(imstk::KeyboardSceneControl)

#ifdef iMSTK_USE_RENDERING_VTK
/*
 * ViewerVTK & RenderingVTK
 */
%shared_ptr(imstk::AbstractVTKViewer)
%shared_ptr(imstk::VTKViewer)
%shared_ptr(imstk::VTKTextStatusManager)
#endif

/*
 * Devices
 */
%shared_ptr(imstk::DeviceClient)
%shared_ptr(imstk::KeyboardDeviceClient)
%shared_ptr(imstk::MouseDeviceClient)
%shared_ptr(imstk::MouseEvent)
%shared_ptr(imstk::DeviceManager)
%shared_ptr(imstk::HaplyDeviceClient)
%shared_ptr(imstk::HaplyDeviceManager)
%shared_ptr(imstk::OpenHapticDeviceClient)
%shared_ptr(imstk::OpenHapticDeviceManager)
%shared_ptr(imstk::VRPNDeviceClient)
%shared_ptr(imstk::VRPNDeviceManager)

#ifdef iMSTK_SYNCHRONOUS_LOGGING
/*
 * Logging for unity
 */
%shared_ptr(imstk::Logger)
%shared_ptr(imstk::LoggerSynchronous)
%shared_ptr(imstk::LogOutput)
%shared_ptr(imstk::NullOutput)
%shared_ptr(imstk::StreamOutput)
%shared_ptr(imstk::CacheOutput)
#endif