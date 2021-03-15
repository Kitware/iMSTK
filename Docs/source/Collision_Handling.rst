Collision Handling
==================

:code:`CollisionHandling` classes in iMSTK defines methods for handling collisions. That is, one may want to take action when a collision happens. This is either an instant response (explicit solution) or something added to a system to solve later (implicitly).

Collision handling methods directly consume collision data which is produced by :code:`CollisionDetection` objects. For this reason, every :code:`CollisionHandling` constructor accepts a :code:`CollisionData` object.

Collision handling happens between a pair of objects. Given this pair the handling has a side. We can either handle the collision for object A, object B, or both at the same time/AB.

With this you will see three strategies then:

::

    imstkNew<CustomSPHCollisionHandlerOneSided> handler(sphObj1, Side::A, collisionData);
    imstkNew<CustomSPHCollisionHandlerOneSided> handler(sphObj1, Side::A, collisionData);
    imstkNew<CustomSPHCollisionHandlerOneSided> handler(sphObj2, Side::B, collisionData);
    imstkNew<CustomSPHCollisionHandlerTwoSided> handler(sphObj1, sphObj2, collisionData);

Generally, having one sided collision handlers is better for modularity, but sometimes it is required to consider both models.

Typical usage in an iMSTK Scene will be done through iMSTK interactions found in the scene. Interactions agglomerate CollisionHandling (how to consume collision data) and CollisionDetection to produce an interaction between SceneObjects (objects on the scene level).

You can read more about interactions here.