# Needles

Needles have physics approximations in iMSTK. There are currently many examples within iMSTK that do implement needles to check out. Amongst others, `PBDTissueSurfaceNeedleContact`, `PBDTissueVolumeNeedleContact`.

<p align="center">
  <img src="media/needleTissue.gif" alt="pbd needle puncture volume"/>
</p>

## Needle

iMSTK currently contains two types of needle definitions.
 - StraigthNeedle: A single line segment needle.
 - ArcNeedle: Define with an arc. Also provides a LineMesh for collision.

 To setup a PBD simulated `StraightNeedle` that is also controlled with a `PbdObjectController` one can do:

 ```cpp
 auto toolObj = std::make_shared<PbdObject>();

auto toolGeometry = std::make_shared<LineMesh>();
auto verticesPtr  = std::make_shared<VecDataArray<double, 3>>(2);
(*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
(*verticesPtr)[1] = Vec3d(0.0, 0.0, 0.25);
auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
(*indicesPtr)[0] = Vec2i(0, 1);
toolGeometry->initialize(verticesPtr, indicesPtr);

toolObj->setVisualGeometry(toolGeometry);
toolObj->setCollidingGeometry(toolGeometry);
toolObj->setPhysicsGeometry(toolGeometry);
toolObj->setDynamicalModel(model);
toolObj->getPbdBody()->setRigid(
    Vec3d(0.0, 1.0, 0.0),         // Position
    1.0,                          // Mass
    Quatd::Identity(),            // Orientation
    Mat3d::Identity() * 10000.0); // Inertia

// Add a component for controlling via another device
auto controller = toolObj->addComponent<PbdObjectController>();
controller->setControlledObject(toolObj);
controller->setLinearKs(20000.0);
controller->setAngularKs(8000000.0);
controller->setUseCritDamping(true);
controller->setForceScaling(0.05);
controller->setSmoothingKernelSize(15);
controller->setUseForceSmoothening(true);
 ```

Add the needle component:

```cpp
// Add a component for needle puncturing
auto needle = toolObj->addComponent<StraightNeedle>();
needle->setNeedleGeometry(toolGeometry);
```

## Punctureable

Needles work in junction with Puncturables. Puncturable exists to keep track of the punctures on a tissue. To define something as puncturable add the component to the `toolObj`.

```cpp
toolObj->addComponent<Puncturable>();
```

## PunctureMap

Both Needle and Puncturable contain a PunctureMap which is a map of the objects punctures. It's assumed:
 - A needle can puncture N different tissues.
 - A needle can puncture the same tissue multiple times.
 - A tissue can be punctured by N needles.
 - A tissue can be punctured multiple times.

### Puncture

A puncture is defined as having a state `REMOVED`, `TOUCHING`, or `INSERTED`. This holds true for all needles. Though sometimes instant puncture needles are desirable, in which case `TOUCHING` would not be used. A puncture also contains a `UserData`. This is not used by iMSTK but tends to be useful in user code. It contains the ids of the puncturable, ids of the cell punctured, and barycentric weights. When puncture occurs a `Puncture` is added to both the `Puncturable` and the `Needle`'s `PunctureMap`. This way each can keep track of what punctures it has. ie: `PunctureMap` is not shared, but individual `Puncture`'s are shared.

## NeedleInteraction

As mentioned `NeedleInteraction` is not yet available in iMSTK but is defined in many iMSTK examples. See [PBDTissueVolumeNeedleContact](Examples/PBDTissueVolumeNeedleContact.md) for one such example. Though the internals differ the user facing API is mostly the same.

```cpp
auto needleInteraction = std::make_shared<NeedleInteraction>(tissueObj, needleToolObj);
...
scene->addSceneObject(needleInteraction);
```

<p align="center">
  <img src="media/needles.png" alt="pbd needle puncture volume"/>
</p>

## Suturing

Suturing is not yet in iMSTK but has been prototyped in an example.

![type:video](./media/pbdSuturing.mp4)