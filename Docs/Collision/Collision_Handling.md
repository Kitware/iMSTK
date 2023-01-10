Collision Handling
==================

:code:`CollisionHandling` classes in iMSTK defines methods for handling collisions. They take input objects (often two) and input collision data. When updated they will consume the collision data to produce appropriate responses. Normally this is a response in a dynamical system. This may either be an instant response at the time of handling or something (such as a constraint) added to the system to solve and produce the response later.

CollisionHandlers and CollisionDetection is normally setup by an interaction through the scene such that the user doesn't have to mess with them unless they want to change parameters of a handler/detection or even subclass them for custom functionalities.

LevelSetCH
--------------------------------

The levelSetCH consumes PointDirection data from a collision detection method to produce impulses/movement in a LevelSet using gaussian kernels. One can alter this kernel size, gaussian sigma. If one chooses to use proportional velocity, then the force of the rigid object is considered in how fast the level set is moved.

While the handler is standalone and can be used in other contexts, it finds its main use in the RigidObjectLevelSetCollision interaction which may be used like so:

::

    imstkNew<RigidObjectLevelSetCollision> interaction(rbdObj, femurObj);
    scene->getCollisionGraph()->addInteraction(interaction);

Should more control be needed one can access the underlying handlers like so:

::

    auto colHandlerA = std::dynamic_pointer_cast<RigidBodyCH>(interaction->getCollisionHandlingA());
    colHandlerA->setUseFriction(false);
    colHandlerA->setBeta(0.05); // inelastic collision

    auto colHandlerB = std::dynamic_pointer_cast<LevelSetCH>(interaction->getCollisionHandlingB());
    colHandlerB->setLevelSetVelocityScaling(0.01);
    colHandlerB->setKernel(3, 1.0);
    colHandlerB->setUseProportionalVelocity(true);

PBDCollisionHandling
--------------------------------

Given two input objects (pbd vs pbd or pbd vs colliding/static) and input collision data it will produce collision constraints in a solver to then be solved later. One of the more complex handlers in iMSTK and handles many cases:

The implementation of this handler is broken into two functions:

* Mesh-Non-Mesh Interactions: The PbdObject is garunteed to be a mesh, but the CollidingObject could be mesh, a primitive, an implicit geometry, etc. The following CD data's are handled:

    * PointIndexDirection-None: Moves the vertex of the mesh (given by index) in the direction and depth specified by the element.
    * Triangle-Vertex/PointDirection: Moves the triangle to the vertex.
    * Edge-Vertex/PointDirection: Moves the edge to the vertex.

* Mesh-Mesh Interactions: In the case both are a mesh the following CD data's are handled:

    * Vertex-Triangle/Triangle-Vertex: Moves the vertex and triangle to just touching.
    * Edge-Edge: Moves the two edges to just touching.
    * Edge-Vertex/Vertex-Edge: Moves the vertex and edge to just touching.

Whats supported then:

* PbdObject vs PbdObject mesh collisions.
* PbdObject vs CollidingObject mesh collisions.
* PbdObject vs CollidingObject primitive collisions.
* PbdObject vs CollidingObject implicit geometry collisions.

The PbdCollisionHandling can be used through the PbdObjectCollision interaction like so:

::

    // Add collision between the two objects with the following collision strategy
    imstkNew<PbdObjectCollision> myCollision(tissueObj, toolObj, "ClosedSurfaceMeshToMeshCD");
    scene->getCollisionGraph()->addInteraction(myCollision);

If more control is needed that is not available in the PbdObjectCollision, you may attain the handler like so:

::

    auto pbdHandler = std::dynamic_pointer_cast<PbdCollisionHandling>(myCollision->getCollisionHandlingA());

When subclassing one may override the base class handling function which is called to consume the collision data or you may override the constraint addition functions. Such functions are useful when custom response (say constraints) is needed upon contact. See PbdTissueSurfaceNeedleContact example for subclassing.


PBDPickingCH
--------------------------------

The PbdPickingCH consumes CD data to constrain PbdObject vertices in contact with an analytical geometry. It constraints them such that their relative positions to the analytical geometry origin and orientation are maintained. It may only be used with PbdObject vs CollidingObject that has an analytical geometry.

The user can call beginPick or endPick to constrain or unconstrain the PbdObject vertices.

The PbdPickingCH can be used through the PbdObjectPicking interaction like so:

::

    // Add picking interaction between a clothObj (PbdObject) and toolObj (with capsule analytical geometry)
    imstkNew<PbdObjectPicking> objectPicking(clothObj, toolObj, "PointSetToCapsuleCD");
    scene->getCollisionGraph()->addInteraction(objectPicking);


RigidBodyCH
--------------------------------

The RigidBodyCH handles collisions between two RigidObject2's or a RigidObject2 and a CollidingObject (where CollidingObject is static/immovable by the rigid object). Currently it only handles mesh to primitive+implicit and primitive to primitive rigid body shapes. It can be used through the RigidObjectCollision interaction like so:

::

    // Create an interaction between a rigid object with a sphere and CollidingObject with plane geometry, resolves undirectionally (always above the plane normal)
    auto rbdInteraction = std::make_shared<RigidObjectCollision>(rbdObjSphere, planeObj, "UnidirectionalPlaneToSphereCD");
    rbdInteraction->setFriction(0.0);
    rbdInteraction->setStiffness(0.0001);
    scene->getCollisionGraph()->addInteraction(rbdInteraction);

Another example using between a rigid OBB and an implicit/SDF geometry:

::

    // Create an interaction between a rigid object with a OBB geometry and CollidingObject with an SignedDistanceField geometry (great for static curved surfaces)
    auto rbdInteraction = std::make_shared<RigidObjectCollision>(cubeObj, planeObj, "ImplicitGeometryToPointSetCD");
    rbdInteraction->setFriction(0.0); // Don't use friction
    rbdInteraction->setStiffness(0.05);
    scene->getCollisionGraph()->addInteraction(rbdInteraction);

SPHCollisionHandling
--------------------------------

The SPHCollisionHandling moves SPH particles out of collision with an object and applies boundary friction to the velocity of the particle.  It currently only works with PointDirection data, of which only primtive collision detection supports. So it is currently limited to implicit and primitive shapes only.

One may use it through the SPHCollision interaction like so:

::

    // Add collision between the two objects (fluidObj is SPHObject and solidObj is a CollidingObject with primitive geometry)
    imstkNew<SphObjectCollision> myCollision(fluidObj, solidObj);
    scene->getCollisionGraph()->addInteraction(myCollision);