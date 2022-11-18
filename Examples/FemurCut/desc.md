# Femur Cut Example

<p align="center">
  <img src="../media/lsmCutting.gif" alt="femur level set cutting"/>
</p>

---

## Description

This example demonstrates rigid-on-level set cutting described in "<em>A level set approach to high fidelity interactive bone shaving for virtual training of surgical osteotomies</em>". A rigid body is controlled with virtual coupling and the bone is eroded through evolution of the level-set near the tool. Continuous collision is used to prevent collision. The level-set is also meshed using marching cubes in chunks. Only chunks modified are updated to keep the example >1000Hz for haptics.

[cpp_insert]: <FemurCutExample.cpp>