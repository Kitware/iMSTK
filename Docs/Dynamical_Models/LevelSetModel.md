# LevelSet Method (LSM)

The level set method can be used to smoothly evolve a PDE over a domain. This could just be distances or could even be something like the heat equation.

The LevelSetModel is currently only implemented to evolve distances for regular 3d grids. It can do so in dense or sparse mode. With dense mode every voxel of the grid is updated every iteration. Whilst the sparse mode accepts a list of impulses to the velocities, evolving only voxels which actually have something to evolve.

## Usage

With it we are able to achieve very fast collision detection and haptic response (>1000hz) and deformation of "statically deformable" object. This finds its uses for bone sawing, burring, drilling.

<p align="center">
  <img src="../media/lsmCutting.gif"/>
</p>

## Code

```cpp
// Setup config
auto lvlSetConfig = std::make_shared<LevelSetModelConfig>();
lvlSetConfig->m_sparseUpdate = true;
lvlSetConfig->m_substeps     = 30;

// Setup the Model
auto model = std::make_shared<LevelSetModel>();
model->setModelGeometry(sdfGeometry);
model->configure(lvlSetConfig);
```

This can be given to a LevelSetDeformableObject for usage in the scene.
