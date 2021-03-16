========
Releases
========

Release 4.0.0
-------------

**Announcement: iMSTK 4.0.0**

This release features major improvements to the simulation execution pipeline, geometry module, virtual reality support, and haptics. Extensive refactoring and numerous bug fixes were also made across the codebase to allow extensibility of certain classes, improve clarity, separate roles for different iMSTK libraries, and enforce consistent design and conventions across the toolkit.

Here is a comprehensive list of changes made for this release.

**New Features**

- Addition of Implicit Geometry
  
  - SignedDistanceField
  - CompositeImplicitGeometry
  - All existing analytical geometries
  
- Addition of LevelSetModel (regular grids only)
- Addition of ImplicitGeometryCD & ImplicitGeometryCCD
- Addition of RigidBodyModel2
- Addition of Event system (addition of EventObject)
- Addition Imstk Data Arrays. DataArray+VecDataArray. Replacement throughout code.
- Addition of per cell and vertex mesh attributes. DynamicalModels use them.
- Addition of PBDPickingCH, for picking PBD vertices.
- New Geometry Filters. Including LocalMarchingCubes.
- Offscreen rendering support through VTK OSMesa
- New substepping and sequential execution mode for SimulationManager

**Improvements or Refactoring**

- SimulationManager and Module Refactor
- Refactor VTKRenderDelegates
- Topology changes supported
- New examples. Many fixed haptics and OpenVR examples.
  
  - FemurCut
  - PBDPicking
  - SDFHaptics
  - FastMarch
  - RigidBodyDynamics2
  - PBDCloth-Remap
  - SPH-Obj-SDFInteraction
  - Vessel
  
- imstkNew
- Refactor geometry base class transforms
- OpenVR, Keyboard, and Mouse Device Refactoring
- Control refactoring
- virtual update and visualUpdate functions for SceneObject
- virtual init and advance functions for Scene
- VRPN build toggle
- Geometry enums replaced with getTypeName polymorphic function
- DynamicalModel enums replaced with getTypeName polymorphic function
- Module unit tests
- VecDataArray + DataArray unit tests
- NRRD, MHD, & NII image file support through VTK
- Debug camera initializes to bounding box of initial scene
- Bounding box computation of many primitives added
- Laprascopic Tool Controller fixed + improved
- VisualObjectImporter can now read and flatten scene hierarchies.
- PBD performance improvements
- HapticDeviceClient accepts no name for default device
- ColorFunctions added to RenderMaterial for mapping scalars
- imstkCamera refactor, view matrix can now be set independently of focal point and position.
- VTKViewer split into VTKViewer and VTKOpenVRViewer, common base VTKAbstractViewer added.
- Mute, log, or display VTK logger options added to VTKAbstractViewer
- Shared RenderMaterials

**Bug Fixes**

- Capsule CD fixes
- OpenVR fixes
- Missing bounding box functions for some analytical shapes added
- Rigid body reset fixes
- Many virtual destructors added

**API Changes**

- OpenVR, Keyboard, and Mouse device refactoring: Mouse and Keyboard now provided under the same DeviceClient API as our haptic devices. You may acquire these from the viewer. They emit events, you can also just ask them about their state.

::

  std::shared_ptr<KeyboardDeviceClient> keyboardDevice = viewer->getKeyboardDevice();
  std::shared_ptr<MouseDeviceClient> mouseDevice = viewer->getMouseDevice();

  std::shared_ptr<OpenVRDeviceClient> leftVRController = vrViewer->getVRDevice

- Controls: Our controls are now abstracted. Any control simply implements a device. You may subclass KeyboardControl or MouseControl. We also provide our own default controls:

::

  // Add mouse and keyboard controls to the viewer
  imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
  mouseControl->setSceneManager(sceneManager);
  viewer->addControl(mouseControl);

  imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
  keyControl->setSceneManager(sceneManager);
  keyControl->setModuleDriver(driver);
  viewer->addControl(keyControl);

- Event System: Key, mouse, haptic, and openvr device event callback can be done like this now.
  
  - You may alternatively use queueConnect as long as you consume it somewhere (sceneManager consumes all events given to it).
  - Your own custom events may be defined in iMSTK subclasses with the SIGNAL macro. See [KeyboardDeviceClient](https://gitlab.kitware.com/iMSTK/iMSTK/-/blob/master/Source/Devices/imstkKeyboardDeviceClient.h) as an example.

::

  connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
    sceneManager, [&](KeyEvent* e)
    {
      std::cout << e->m_key << " was pressed" << std::endl;
    });

- Imstk Data Arrays: Data arrays and multi-component data arrays provided. They are still compatible with Eigen vector math.

::

  VecDataArray<double, 3> myVertices(3);
  myVertices[0] = Vec3d(0.0, 1.0, 0.0);
  myVertices[1] = Vec3d(0.0, 1.0, 1.0);
  myVertices[2] = myVertices[0] + myVertices[1];

  std::cout << myVertices[2] << std::endl;

- SimulationManager may now be setup and launched as follows:

::

  // Setup a Viewer to render the scene
  imstkNew<VTKViewer> viewer("Viewer");
  viewer->setActiveScene(scene);

  // Setup a SceneManager to advance the scene
  imstkNew<SceneManager> sceneManager("Scene Manager");
  sceneManager->setActiveScene(scene);
  sceneManager->pause(); // Start simulation paused

  imstkNew<SimulationManager> driver;
  driver->addModule(viewer);
  driver->addModule(sceneManager);
  driver->start();

- `VisualObject` typedef removed. Just use `SceneObject`.
- `HDAPIDeviceServer` renamed to `HapticDeviceManager`
- `HDAPIDeviceClient` renamed to `HapticDeviceClient`

**Contributors**

Andrew Wilson,
Venkata Sreekanth Arikatla,
Ye Han,
Harald Scheirich,
Bradley Feiger,
Jianfeng Yan,
Johan Andruejol,
Sankhesh Jhaveri


Release 3.0.0
-------------

**Announcement: iMSTK 3.0.0**

This release features major improvements to the computational workflow, physics, and rendering aspects of the toolkit. Major refactoring and bug fixes were made across the board to allow easy extension of classes, improve clarity and separation of roles of different imstk libraries and enforce consistency of design across the toolkit.

Here is a comprehensive list of changes made for this release.

**New Features**

- Introduction of configurable task-graph and task-based parallelism.
- Major upgrade to the rendering module (VTK backend)

  - Upgrade to VTK 9.0
  - Realistic fluid rendering using screen space fluids
  - Faster particular rendering of fluids
  - Addition of physically based rendering
  
- Addition of 3D image support and volume rendering
- Improved physics models for particle based dynamics: Addition of extended position based dynamics (xPBD)
- Addition of support for modeling 1D elastic structures with bending stiffness
- Addition of faster reduced order deformation models (Linux only)
- Addition of Reverse Cuthill–McKee algorithm (RCM) for mesh renumbering
- Major refactoring simulation manager: Improved time stepping policies, multiple scene management and scene controls, addition of async simulation mode
- Improved capabilities of the geometric utility module: addition of geometric processing filters, New tetrahedral mesh cover generation (based on ray-casting)

**Improvements or Refactoring**

* Upgrade external dependency from Vega 2.0 to 4.0 (finite element library backend)
* Clear majority of the warnings in imstk libraries
* Refactored examples: consistent naming, factoring out object addition into separate functions, use heart dataset, remove redundant mapping, Removed line mesh example
* New examples for scene management, volume rendering, task graph
* Renamed files to be consistent with class names
* Vulkan shader project removed for VTK backend
* Remove imstkVolumetricMesh dependency on vega volumetric mesh
* Easy configuration of finite element deformable object, viewer, renderer and simulation manager
* Concrete dynamcal models now derive from AbstractDynamicalModel
* Solvers are moved to models from scene
* Added default solvers for models
* SPHSolver is removed
* SceneObject class now has update calls
* DynamicalObject de-templatized
* Fix render window default title to imstk
* Replace external project download links with .zip versions
* Uses CHECK() instead of LOF(FATAL)/LOG_IF(FATAL) for simplicity
* imstkLogger is now a singleton
* Allow exclusion of files while building library targets
* Refactoring to use forward declarations where possible
* Templated solvers with matrix type
* Faster TetraToTriangle map
* Interactions are now specified explicitly
* PbdConstraints moved to Constraints library, PbdConstraints and PbdModel decoupled
* PbdModel performance improvements
* SPHModel performance improvements (using TaskGraph)

**Bug Fixes**

* Fix PhysX backend build issues on Ubuntu
* Fix imstkFind.cmake issues
* Fix imstkConfig.cmake issues
* PbdModel reset fix
* All Scene, SceneObjects reset correctly now

**API Changes**

* simulationManager::startSimulation() to simulationManager::start()
* CollisionGraph::addInteraction(std::shared_ptr<CollidingObject>, std::shared_ptr<CollidingObject>, CollisionDetection::Type, CollisionHandling::Type, CollisionHandling::Type) to CollisionGraph::addInteraction(std::shared_ptr<SceneObjectInteraction>())
* DynamicalModels now have default solvers

**Contributors**
Venkata Sreekanth Arikatla,
Andrew Wilson,
Jianfeng Yan,
Aaron Bray,
Sankhesh Jhaveri,
Johan Andruejol



Release 2.0.0
-------------

**Announcement: iMSTK 2.0.0**

This release adds major features for the physics and rendering modules. Parallel
support is also added. Major improvements to the CMake build and install steps have been
implemented. Many modules have been refactored for clarity and to reduce reducdency.

For more information, visit our website: http://www.imstk.org/

**New Features**

- Rigid body dynamics with Physx backend
- Debug rendering support
- Octree-based collision detection
- Multithreading support (using Intel TBB)
- Smoothed Particle Dynamics for fluids
- Customizable on-screen text
- New simulation modes for simulation manager to allow flexibility
- VR support for Vulkan backend
- Particle systems  for visual effects
- Lens distortion for use in VR (Vulkan backend)
- Vulkan renderer compressed texture support

**Improvements or Refactoring**

- Improved CMake build and install
- Enable compiler flags to report W4-level warnings 
- Remove cyclic dependencies between modules
- Add color to stdout on windows
- Refactored Position based dynamics classes
- Refactor rendering specification using visual model
- Modifications to the code formatting rules
- Refactor geometry mapping classes
- Remove unused files and classes
- Disable building tests for external dependencies
- Update the vrpn to the latest to fix linux build
- Update VTK backend to 8.2.0
- Remove ODE external library

**Bug Fixes**

- Fix undefined behaviour of PBDModelConfig
- Use vtkPolyData instead of vtkPolyLine for VTKdbgLinesRenderDelegate
- Fix compilation with BUILD_EXAMPLES Off


**Contributors for this release**

Venkata Sreekanth Arikatla,
Nghia Truong,
Nicholas Boris Milef,
Aaron Bray,
Ruiliang Gao,
Johan Andruejol



Release 1.0.0
-------------

**Announcement: iMSTK 1.0.0**

We are introducing Interactive Medical Simulation Toolkit (iMSTK)-a free & open source software toolkit written in C++ that aids rapid prototyping of interactive multi-modal surgical simulations.

For more information, visit our website: http://www.imstk.org/

**Features**

- Cross-platform build
- CMake automated superbuild
- Test infrastructure (via google test)
- Continuous Integration
- Scene and simulation management
- Vulkan and VTK rendering backends
- Advanced rendering: Physically based rendering, Subsurface scattering, Decals, Shadows, 
- Graphical overlays (Vulkan backend only)
- Standard user controls (pause, run, exit, pan-zoom-rotate)
- SteamVR support including (Oculus, HTC Vive (VTK backend only)
- Finite elements (linear, co-rotational, non-linear formulations)
- Position based dynamics
- Penalty and constraint-based collision handling
- Linear solvers: Direct and Iterative matrix solvers
- Non-linear Newton solver
- Collision detection (CCD, Spatial hash based collision, narrow phase queries)
- External device support (VRPN)
- Support for standard mesh input formats (.obj, .dae, .fbx., .stl, .vtk, .vtu, etc.)
- Asynchronous logging (using g3log)
- Audio support
- Haptic rendering (OpenHaptics)

**Contributors for this release**

Venkata Sreekanth Arikatla,
Alexis Girault,
Nicholas Boris Milef,
Ricardo Ortiz,
Thien Nguyen,
Rachel Clipp,
Mohit Tyagi,
Samantha Horvath,
Jean-Baptiste Vimort,
Sean Radigan,
David Thompson,
Dženan Zukić,
Mayeul Chassagnard,
Tansel Halic,
Hina Shah,
Andinet Enquobahrie,
Hong Li,
Shusil Dangi