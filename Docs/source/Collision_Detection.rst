Collision Detection
===================

Any collision detection class in iMSTK is given by the subclasses to :code:`CollisionDetection` base class. These produce CollisionData. CollisionData is very heterogenous. But generally, it will describe how one geometry can be moved such that it isn't colliding anymore. Often this comes in the form of contacts. For instance, the :code:`PenetrationCollisionDataElement` simply gives a penetration vector. How this vector is computed is up to the collision detection algorithm/method. Different algorithms/methods are used for differing models and geometries. Additionally, there may be multiple suitable methods for the same models and geometries.

Commonly used collision detection classes in iMSTK are

::

    SurfaceMeshToSurfaceMeshCD
    SurfaceMeshToSurfaceMeshCCD
    ImplicitGeometryToPointSetCD
    ImplicitGeometryToPointSetCCD
    MeshToMeshBruteForceCD

Beyond those its mostly primitive collision methods and volumetric mesh collision methods. Outside of iMSTK, if one wants to use collision detection as a standalone library, one can set up like this:

::

    auto surfMesh1 = MeshIO::read<SurfaceMesh>(...);
    auto surfMesh2 = MeshIO::read<SurfaceMesh>(...);
    imstkNew<CollisionData> collisionData;
    imstkNew<MeshToMeshBruteForceCD> cd(surfMesh1, surfMesh2, collisionData);

    // Calling this will fill collisionData
    cd->computeCollisionData();

Typical usage in an iMSTK Scene will be done through iMSTK interactions found in the scene. Interactions agglomerate CollisionHandling (how to consume collision data) and CollisionDetection to produce an interaction between SceneObjects (objects on the scene level).

You can read more about interactions `here <https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/Source/Scene/imstkObjectInteractionPair.h>`__.