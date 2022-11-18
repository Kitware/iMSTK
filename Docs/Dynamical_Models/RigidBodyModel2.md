# Rigid Body Dynamics (RBD)

iMSTK provides its own implementation of a rigid body model. However, this has been replaced with our PBD model which now does all rigid bodies, soft bodies/deformables, and fluids. Please use PBD instead.

The `RigidBodyModel2` in iMSTK is a linear constraint based system. It is solved with projceted-gauss-seidel (PGS). The constraints are polymorphic meaning it is easy to extend. This system solves for impulses/instant changes in velocities to separate bodies in contact.

## Usage

Rigid body simulations in iMSTK mostly find their use with tools.

<p align="center">
  <img src="../media/rbd1.gif"/>
</p>

## Code

Unlike PBD, FEM, SPH the RigidBodyModel & RigidBodyModel2 deal with multiple differing geometries in the same model for implicit solves. RigidBodyModel2 can be configured like so:

```cpp
// This model is shared among interacting rigid bodies
auto rbdModel = std::make_shared<RigidBodyModel2>();
rbdModel->getConfig()->m_gravity = Vec3d(0.0, -2500.0, 0.0);
rbdModel->getConfig()->m_maxNumIterations = 10;

std::shared_ptr<RigidBody> body1 = rbdModel->getRigidBody();
body1->m_mass = 100.0;
body1->m_initPos = Vec3d(0.0, 8.0, 0.0);
body1->m_initOrientation = Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0)));
body1->m_inertiaTensor = Mat3d::Identity();

std::shared_ptr<RigidBody> body2 = rbdModel->getRigidBody();

... body 2 setup ...
```

However, if using a RigidObject2 in the scene it will create its RigidBody, usage then looks like the following:

```cpp
// This model is shared among interacting rigid bodies
imstkNew<RigidBodyModel2> rbdModel;
auto rbdModel = std::make_shared<RigidBodyModel2>();
rbdModel->getConfig()->m_gravity = Vec3d(0.0, -2500.0, 0.0);
rbdModel->getConfig()->m_maxNumIterations = 10;

// Object setup for the scene
auto cubeObj = std::make_shared<RigidObject2>();
cubeObj->setDynamicalModel(rbdModel);
cubeObj->setPhysicsGeometry(subdivide->getOutputMesh());
cubeObj->setCollidingGeometry(subdivide->getOutputMesh());
cubeObj->addVisualModel(visualModel);

// We can deal with the rigid body properties like so
cubeObj->getRigidBody()->m_mass    = 100.0;
cubeObj->getRigidBody()->m_initPos = Vec3d(0.0, 8.0, 0.0);
cubeObj->getRigidBody()->m_initOrientation = Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0)));
cubeObj->getRigidBody()->m_intertiaTensor  = Mat3d::Identity();
```