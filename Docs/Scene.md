# Scene

## Overview

The `Scene` defines a flat collection of `SceneObject`s and fully represents the virtual environment. `SceneObject`s may be a surgical tool, a tissue, OR table, a leg, light, or even non-visual objects. A simple visual object can be added to the `Scene` as:

```cpp
auto scene = std::make_shared<Scene>("MyScene");

auto myObject = std::make_shared<SceneObject>("MyObject");
auto visuals = myObject->addComponent<VisualModel>();
// visuals->setGeometry(<my geometry here>);
scene->addSceneObject(myObject);
```

iMSTK contains an entity-component (ECS) model that is still a work in progress. Whilst objects/entities represent whole concepts such as a `tool` or `table`. A component represents a **swappable** part of the object/entity such as the legs of a table or reciprocation of a saw tool. ECS's are common in many game engines for modularity and flexibility. One may subclass Component or Behaviour to add new functionality.

## Entity/SceneObject

Whilst iMSTK is in the middle of an ECS refactor. The SceneObject represents and object in the scene. It stems from Entity providing an optional visual geometry and a `TaskGraph` to introduce `TaskNode` steps to the scene `TaskGraph` that is run during advacement of the scene.

Many types stem from SceneObject eventually to be moved to `Component`s, here's a full list of those types:

- **SceneObject**: Provides an object with a visual geometry and empty virtual update function. Geometry is optional.
- **CollidingObject**: Provides an object with both visual and colliding geometry.
- **PbdObject**: Provides a PbdModel governing its physics.
- **RigidObject**: Provides a RigidBodyModel governing its physics.
- **RigidObject2**: Provides a RigidBodyModel2 governing its physics. 
- **FemDeformableObject**: Provides a FemModel governing its physics.
- **SPHObject**: Provides a SPHModel governing i
- **LevelSetDeformableObject**: Provides a LevelSetModel governing its physics.

### Component

An entity (`SceneObject` base) is made of many components. A component represents a data-only part of an entity. It does no function, has an initialization, and a reference to the entity it is part of. Components can be placed on/off entities/objects whenever. One can extend to hold relevant data and use elsewhere.

```cpp
class MyComponent : public Component
{
public:
    Mat4d myTransform = Mat4d::Identity();
};
```

### Behaviour

A `Behaviour` is a `Component` that has function. Generally only recommend for small independent bits of logic. Anything that involves multiple components, call order, lots of communication, or multiple `Entity`'s may need its own system such the VTKRenderer or Scene. Alternatively the TaskGraph can be used.

```cpp
class MyBehaviour : public Behaviour
{
public:
    void update() override {} // Called per SceneManager::update (Scene::advance)
    void visualUpdate() override {} // Called per VTKViewer::update
};
```

It is safe to assume the physics system is complete before entering any components. See more in the computational flow section.

## Initialization of the Scene

Initialization of the scene and the things in it is done as follows:

- Initialize `Scene`
    - Initialize all `SceneObject`'s of the `Scene`
    - Initialize all `Component`'s of every `SceneObject` in the `Scene`
        - If any `Component`s are introduced during `Component::initialize`, they are initialized too
    - Initialize all the `DynamicalModel`'s used by `SceneObject`'s in the `Scene`
    - Build the `TaskGraph` of the `Scene`

## Advancement of a Scene

Normally the `Scene` would be given to the `SceneManager` to run and `VTKViewer` to render. These would be apart of `SimulationManager`. However, one is able to advance the scene at their own timestep by using:

```cpp
scene->initialize(); // Must be initialized first
scene->advance(0.001); // Timestep to advance by
```

Update order is as follows:
- Update all `SceneObject`s and `Behaviour`s on them.
- Update all `TaskGraph` nodes (including physics steps).