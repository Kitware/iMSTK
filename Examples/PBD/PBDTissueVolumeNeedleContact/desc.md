# PBD Tissue Volume Needle Contact

<p align="center">
  <img src="../media/needles.png" alt="tissue volume needle contact example"/>
</p>

---

## Description

This example demonstrates needle contact and puncture with tetrahedral meshed tissues. A organ, meshed with TetWild, is setup along with another skin layer. Both are simualted with PBD-based St. Venant Kirchoff model. A needle is also simualted with PBD rigid bodies and given a single line element as a collision geometry. 
The tool is controlled with virtual coupling.

```cpp
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

Finally two `NeedleInteraction` are setup with the organ+skin tissues to the rigid tool:

```cpp
// This adds both contact and puncture functionality
auto interaction = std::make_shared<NeedleInteraction>(tissueObj, toolObj);
interaction->setPunctureForceThreshold(3.0);
interaction->setNeedleCompliance(0.000001);
interaction->setFriction(0.1);
scene->addInteraction(interaction);
// This adds both contact and puncture functionality
auto interaction2 = std::make_shared<NeedleInteraction>(tissueObj2, toolObj);
interaction2->setPunctureForceThreshold(3.0);
interaction2->setNeedleCompliance(0.000001);
interaction2->setFriction(0.1);
scene->addInteraction(interaction2);
```

With `NeedleInteraction` the needle is allowed to transition from a collision to puncturing state after the force threshold is exceeded. Once transitioned, collisions are disabled until the needle is removed again. Once punctured the embedding constraints kick in and constraint the needle to the original puncture locations on the triangles via the initial barycentric coordinates when it entered that triangle.

[cpp_insert]: <PBDTissueVolumeNeedleContactExample.cpp>