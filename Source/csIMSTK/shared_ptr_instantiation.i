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
%shared_ptr(imstk::VecDataArray<int, 3>)
%shared_ptr(imstk::VecDataArray<int, 4>)
%shared_ptr(imstk::VecDataArray<float, 2>)
%shared_ptr(imstk::VecDataArray<double, 2>)
%shared_ptr(imstk::VecDataArray<double, 3>)
%shared_ptr(imstk::VecDataArray<unsigned char, 3>)
%shared_ptr(imstk::ModuleDriver)
%shared_ptr(imstk::EventObject)

/* 
 * Geometry 
 */
%shared_ptr(imstk::Geometry)
%shared_ptr(imstk::PointSet)
%shared_ptr(imstk::ImageData)
%shared_ptr(imstk::LineMesh)
%shared_ptr(imstk::PointSet)
%shared_ptr(imstk::SurfaceMesh)
%shared_ptr(imstk::VolumetricMesh)
%shared_ptr(imstk::TetrahedralMesh)
%shared_ptr(imstk::HexahedralMesh)
%shared_ptr(imstk::ImplicitGeometry)
%shared_ptr(imstk::AnalyticalGeometry)
%shared_ptr(imstk::CompositeImplicitGeometry)
%shared_ptr(imstk::Plane)
%shared_ptr(imstk::Sphere)
%shared_ptr(imstk::Cube)

/* 
 * Geometry 
 */
%shared_ptr(imstk::GeometryMap)
%shared_ptr(imstk::OneToOneMap)
%shared_ptr(imstk::TetraTriangleMap)

/* 
 * DynamicalModel 
 */
%shared_ptr(imstk::PbdObject)
%shared_ptr(imstk::PBDModelConfig)
%shared_ptr(imstk::PbdCollisionConstraintConfig)
%shared_ptr(imstk::PbdFEMConstraintConfig)
%shared_ptr(imstk::PbdSolver)
%shared_ptr(imstk::FeDeformBodyState)
%shared_ptr(imstk::FEMModelConfig)
%shared_ptr(imstk::FEMDeformableBodyModel)
%shared_ptr(imstk::AbstractDynamicalModel)
%shared_ptr(imstk::DynamicalModel<imstk::PbdState>)
%shared_ptr(imstk::DynamicalModel<imstk::FeDeformBodyState>)
%shared_ptr(imstk::RigidBodyState2)
%shared_ptr(imstk::DynamicalModel<imstk::RigidBodyState2>)
%shared_ptr(imstk::PbdModel)
%shared_ptr(imstk::expiremental::RigidBodyModel2Config)
%shared_ptr(imstk::expiremental::RigidBodyModel2)
%shared_ptr(imstk::TimeIntegrator)
%shared_ptr(imstk::BackwardEuler)

/* 
 * Rendering
 */
%shared_ptr(imstk::RenderMaterial)
%shared_ptr(imstk::Texture)

/*
 * Constraint
 */
%shared_ptr(imstk::expiremental::RigidBody)

/* 
 * SceneEntities
 */
%shared_ptr(imstk::SceneObject)
%shared_ptr(imstk::CollidingObject)
%shared_ptr(imstk::DynamicObject)
%shared_ptr(imstk::PbdObject)
%shared_ptr(imstk::FeDeformableObject)
%shared_ptr(imstk::expiremental::RigidObject2)
%shared_ptr(imstk::VisualModel)
%shared_ptr(imstk::Camera)
%shared_ptr(imstk::Light)
%shared_ptr(imstk::DirectionalLight)
%shared_ptr(imstk::PointLight)
%shared_ptr(imstk::SpotLight)

/*
 * CollisionDetection
 */

/*
 * CollisionHandling
 */
%shared_ptr(imstk::CollisionPair)
%shared_ptr(imstk::CollisionHandling)
%shared_ptr(imstk::expiremental::RigidObjectCollidingCollisionPair)
%shared_ptr(imstk::expiremental::RigidBodyCH)

/*
 * Controller
 */
%shared_ptr(imstk::DeviceControl)
%shared_ptr(imstk::MouseControl)
%shared_ptr(imstk::KeyboardControl)

/*
 * Scene
 */
%shared_ptr(imstk::Scene)
%shared_ptr(imstk::CollisionGraph)
%shared_ptr(imstk::InteractionPair)
%shared_ptr(imstk::ObjectInteractionPair)

/*
 * SimulationManager
 */
%shared_ptr(imstk::Module)
%shared_ptr(imstk::Viewer)
%shared_ptr(imstk::AbstractVTKViewer)
%shared_ptr(imstk::VTKViewer)
%shared_ptr(imstk::SceneManager)
%shared_ptr(imstk::SimulationManager)
%shared_ptr(imstk::MouseSceneControl)
%shared_ptr(imstk::KeyboardSceneControl)

/*
 * Devices
 */
%shared_ptr(imstk::DeviceClient)
%shared_ptr(imstk::KeyboardDeviceClient)


