========
Releases
========

Release 5.0.0
-------------

**Announcement: iMSTK 5.0.0**

This release features major improvements to the collision detection and response capabilities, rendering, and hardware interface modules. Numerous refactors and bug fixes were made across the board to improve extensibility. The testing infrastructure and coverage was improved substantially. Numerous medically relevant examples were added. Most notably, v5.0 comes with a beta version of the C and C# wrappers for iMSTK along with documentation and examples.

Here is a comprehensive list of changes made for this release.

**Andrew Wilson**

**Enhancements**

- Extended PBD model to allow needle-tissue interactions (`07819c60 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/07819c608d14e9fbd01b60b37ab77bf99ee9f592>`_)

  - PBDTissueSurfaceNeedleContact: Simple approach to needle insertion of deformable 2d triangle mesh tissue with rigid body haptics.
  - RbdSDFNeedle: Simple approach to needle insertion of static tissue for which a rigid body needle may rotate/pivot before being fully inserted.
  - PBDTissueVolumeNeedleContact: More involved approach to needle insertion of deformable tetrahedral mesh tissues with rigid body haptics. Uses two-way embedded coupling.
  - PBDStaticSuture: Suture thread and needle vs static geometry. Uses custom arc to point constraint. Suture thread non functional yet. Only surface constrained.

- The collision detection architecture was comprehensively refactored and updated (`ed212e8f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ed212e8fcc66d9cbb34e6022569cce679fab8b2c>`_)

  - PbdObject vs CollidingObject possible: PbdObject vs Rigid, SPH, FEM, any CollidingObject are now possible.
  - Pbd vs Primitives Collision: PbdCollisionConstraint’s and PbdCollisionHandler now use pointers and values instead of DataArray's allowing collision between something that doesn’t have a   vertex buffer.
  - PbdPointToEdgeConstraint: Allows mesh to curved surface collision when used together with point-to-point and point-to-triangle.
  - MeshToMeshBruteForceCD New Implementation: Able to resolve deep contacts in manifold shapes. Stable and accurate method, not performant for large meshes.
  - Virtual constraint addition for Pbd and Rbd models: Allows redefinition of contact response models. Used for needles, drills, thread holes, cutting, etc.
  - PbdRigidCollision interaction: Two-way collision between RigidObject2 and PbdObject. Gives response between both objects. (useful for haptics, no reaction force on the RigidObject2 is felt without it). See PBDTissueContactExample.
  - CollisionDebugObject: Can be added to the scene to display CollisionData as faces, edges, points, or directions/arrows.
  - SurfaceMeshToSphereCD: Triangle mesh to sphere collision. Demonstrated in PbdClothCollisionExample & RbdSurfaceMeshToSphereCDExample.
  - Restitution and friction added to PbdModel

- Light properties can be changed at runtime, attenuation exposed (`2d9a78c1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2d9a78c1458d36cf6e2f291ca488e1ccd81dd014>`_)
- Textures can now be setup via ImageData instead of filenames, swapped at runtime, & pixels modified at runtime (`6e79fb81 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6e79fb81eafc24f667c859ed02eadbe712474c1d>`_)
- VisualModels can be added/remove from the scene at runtime. (`f1e55106 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f1e551066c6f3356197ab980803a9b43f3df5b7c>`_)
- Camera now provides projection matrix and eye rays (`bdbffbb8 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bdbffbb8d704b923fa42a2ee8381dc691a143936>`_)
- Added OrientedBox to geometry (replaces Cube) (`845dd6c5 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/845dd6c5c691d942b555bb31f380699d53aa9299>`_)
- PbdConstraintFunctor was added that allows customization of constraints generated for a PbdModel. (`e3a9753f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e3a9753f18673192defb37c9f66246f94463c673>`_)
- Added strides to PBD bending model. Bend constraints can now be generated for differing strides allowing stiffer threads/strings at lower iterations. (`350adc5f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/350adc5fd574e7beaa67193512b5a207092ebe1a>`_)
- Added RenderMaterial::ShadingModel::None allowing shading to be turned off where necessary (`e7811080 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e7811080094426f032ecf942d5be5c1a778f20df>`_)
- Added RenderMaterial::DisplayMode::Normals that displays normal directions using arrows (`614e296e <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/614e296eac148c8a096e2a9199bf223bdcb0783c>`_)
- Added Texture Projection filter that projects attributes (eg: uv coords) via closest point projection (`17fb7679 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/17fb767916171cc5d3ac0329d33517d7252ab600>`_)
- Cell attributes were added to LineMesh (`08b08212 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/08b082120d29f84a7d6bf925398e2eb40deadec6>`_)
- LineMeshes can be read in via VTK file type (`6e20e88b <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6e20e88bf08554417ddcaf7c29002c50bf810114>`_)
- SceneObject::visualUpdate virtual function is now called before every render for every SceneObject in the scene (`fa95ff5f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fa95ff5f9523a519c4b4f1e506ff55d39f1e16c7>`_)
- Added disposable syringe, orthopedic drill, c6 needle 3d models to the imstk data
- Performance improvements to SPH (`6a2bb089 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6a2bb0893a3651f398c35c1e858418bf1fc7e525>`_)
- Performance improvements to Dense LSM (`d2a2b88f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d2a2b88f75366cc7bbba1dcf8a46efc48bf497d1>`_)
- Remove nonfunctional Pulse (`3b41b3e1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3b41b3e10718192778b86bec0a98fdae586f51cd>`_)
- Remove nonfunctional Vulkan (`5e7eddbc <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5e7eddbc97fa141eba78cb5921d94c6f258cb316>`_)

**Documentation Updates**

- PbdModel documentation updated (`6eb4a04a <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6eb4a04a78118b9e501e5b48d336697e522271cb>`_)
- CollisionDetection documentation updated (`2d9a78c1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2d9a78c1458d36cf6e2f291ca488e1ccd81dd014>`_)

**Testing**

- PbdBendConstraintFunctor testing (`dc431620 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dc431620c972ec165964dc513956754a7a7edca6>`_)
- CollisionDetection testing (`07819c60 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/07819c608d14e9fbd01b60b37ab77bf99ee9f592>`_), `3b19c782 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3b19c782b4b33349f84cdebd58f0c176e20a2f30>`_, `91d09f63 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/91d09f6308b994af8dd3731f1ce28ec8244bf8a1>`_)
- PbdCollisionConstraint testing (`ed212e8f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ed212e8fcc66d9cbb34e6022569cce679fab8b2c>`_)
- PGSSolver testing (`567f126e <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/567f126e61ecbd44946f96aa10eef5505e8be8ad>`_)

**Harald Scheirich**

**Enhancements**

- VRPN Analog, Button and Tracker devices. No limit to VRPN in iMSTK anymore (co-authored by Khalil Oumimoun) (`0f02a666 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0f02a666a2879cc8ac81be6737fb30b6a94e533e>`_)
- Improved testing capabilities. Reduced overhead in testing infrastructure removing auto generated classes. (`3059ffbe <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3059ffbe1cf91300650732b76461a0135efa3a54>`_)
- Added Git LFS-based external data management for data required by tests and examples. (`7eb3d176 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7eb3d17684368fcf59496bb288a57288401b568f>`_)
- Update gtest (`a4d30ec5 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a4d30ec53b828396a1c744bef4345e9130041736>`_)

**Documentation Updates**

- Improve C# Wrapper Documentation (`28050bd3 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/28050bd3b9d92d7accd914c6be95594da27d64b4>`_)
- Added documentation for external Data and dependency update process (`a4051249 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a40512497e61e1bb79ccb442cb3f3c23b5911e6d>`_)
- Added coding style guidelines (`1f450dcf <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1f450dcf07cb3a02a97d0f1e90984888fcb3bb4e>`_)

**Testing**

- DataArray testing (`99f18c24 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/99f18c2452441f58742879c9e7a6a42b03819651>`_, `8b08c873 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8b08c87378f5297e10a1360d43156c1476ce2aba>`_)
- Device & Control testing (`1b76fed9 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1b76fed9eb8e3aadc6a28872969badf50e32fb4a>`_)
- EventObject testing (`538915c8 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/538915c8aaa3a52ce5772d12a87eceb5ead53cd8>`_)
- GeometryAlgorithm testing (`d061764a <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d061764a4e22dfab69ea041fbb9fe122bc2e9e80>`_)
- SurfaceMesh tests (`e7dfcb23 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e7dfcb23c4c75b47a5cde5987b84e3c82872b854>`_)
- Render testing (`57725ee0 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/57725ee0f06aabf7046a9c376affa8c16e3f8471>`_)

**Sreekanth Arikatla**

**Enhancements**

- Remove plotter utils (`5508d197 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5508d197b171651c4da24e011d770bc703afb71e>`_)
- Remove APIUtils (`554cde05 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/554cde055ef21dba9fb2941341146414062b658a>`_)
- Cleanup Vulkan references and remove related external dependencies imgui, glm (`d6cea42b <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d6cea42bc4bff9e7f549d12bf161c2769019039a>`_)
- Scene bounding box computation (`458fa955 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/458fa955f82a7f81d4f96c5b333045ecb64369a0>`_)

**Testing**

 - PbdConstraintFunctor testing (`a55301f1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a55301f123dda66bc9e01e452e3e82f420427c75>`_)
 - Geometry testing (`613402e1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/613402e183d7f9d842cdc8e1c3216c7d528a79e1>`_, `84caab04 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/84caab046b3b95964d2e1ee1ada6222c8ef00a24>`_)
 - CollisionDetection testing (`07819c60 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/07819c608d14e9fbd01b60b37ab77bf99ee9f592>`_, `3a135f83 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3a135f8397571e21553a358a7f82b78bcbe58b60>`_, `6b5a3685 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6b5a3685eb7314ae165cbdb7406087b829091785>`_, `2e6ad2aa <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2e6ad2aaa4c7362e6a4aaea51544d017e5047c0b>`_, `266ea1b3 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/266ea1b3084b8d2ad49a588d52b4d395d474a145>`_, `64363cd1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/64363cd1d52867d3dd263cb75af82c9c52f81a54>`_, `2f9e385f <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2f9e385ff6ae03947c3e16761ba13af6f94dc292>`_, `666467bd <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/666467bdced2455f76829963189f6322150b115d>`_)
 - Scene testing (`cfdbe9e3 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cfdbe9e3361b4103e84000c5b7b697bd70ab6919>`_)

**Infrastructure**

- Enabled nightly readTheDocs build.

**Jianfeng Yen**

**Enhancements**

- SWIG-based generation of C and C# wrappers along with C# examples and tests (See Source/Wrappers/csharp) (`c11d712c <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c11d712c3098f0be56ca38f739784c326c445edb>`_)
- Performance Improvements to FEM (`c9f002ec <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c9f002ecfe402e78677ee43146c23d5875aad160>`_)

**Ye Han**

**Enhancements**

- Triangular mesh cutting via local remeshing (`e7c01ead <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e7c01eadc4b73a0fa43c1ed2f6d98f5c48844d72>`_)
- Added SurfaceMeshToCapsuleCD static collision method (`42a321fb <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/42a321fb9285fc07eb24ece219b131015b2f2ba9>`_)

**Khalil Oumimoun**

**Enhancements**

- VRPN Analog, Button and Tracker devices (`0f02a666 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0f02a666a2879cc8ac81be6737fb30b6a94e533e>`_)

**Aron Bray**

**Testing**

- Integration tests for rendering and geometry modules (`96f084a1 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/96f084a1ed0445c3c9cfc6d26438ceb0f2344fb7>`_)

**Hong Li**

**Enhancements**

- Extension of PBD Constraints to inflate tissue (`3bf08161 <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3bf0816139102ad4fb901695263f0d2b4a51a458>`_)

**Furkan Dinc**

**Enhancements**

- Screen space ambient occlusion support (`ab38797d <https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ab38797d496bbb449763b5fa56da41276bd27c4a>`_)

**Ben Boeckel**

**Infrastructure**

- Added Linux to the merge request builds
-	Fixed issue on the MSVC 2017 merge request build

**Api Changes**

 - `PbdPointDirectionConstraint` replaced with `PbdPointToPointConstraint`.
 - `PbdCollisionConstraint::initConstraint` now use `VertexMassPair` structs for initialization.
 
::

  PbdPointPointConstraint constraint;
  constraint.initConstraint(
      { vertexA, invMassA, vertexVelocityA },
      { vertexB, invMassB, vertexVelocityB },
      stiffnessA, stiffnessB);
 
 - `PbdCollisionConstraint::projectConstraint` updated, accepts no parameters anymore as it uses pointer values provided during `PbdCollisionConstraint::initConstraint`.
 - `PbdCollisionSolver::addConstraint` now only accepts a constraint, no buffers.
 
::

  std::vector<PbdCollisionConstraint*>* constraints;
  // ...Fill out constraints...
  myPbdCollisionSolver->addCollisionConstraints(constraints);
 
 - `CollisionDetection` replaced with `CollisionDetectionAlgorithm`. Now subclasses `GeometryAlgorithm` with inputs given via `GeometryAlgorithm::setInput`.
 
::

  // Either order is ok (mesh, sphere) or (sphere, mesh)
  imstkNew<SurfaceMeshToSphereCD> collisionDetect;
  collisionDetect->setInput(mySurfMesh, 0);
  collisionDetect->setInput(mySphere, 1);
  collisionDetect->update();
  
  // Output order dependent on input order
  collisionDetect->getCollisionData()->elementsA; // CD elements for the mesh
  collisionDetect->getCollisionData()->elementsB; // CD elements for the sphere
 
 - Collision interactions renamed, more consistent naming, removal of "Pair".
   - `PbdObjectCollisionPair` renamed to `PbdObjectCollision`.
   - `SphObjectCollisionPair` renamed to `SphObjectCollision`.
   - `RigidObjectLevelSetCollisionPair` renamed to `RigidObjectLevelSetCollision`.
   - `RigidObjectCollisionPair` renamed to `RigidObjectCollision`.
 - `PbdObjectCollision` can now be constructed with both PbdObject+PbdObject or PbdObject+CollidingObject
 
::

  imstkNew<PbdObjectCollision> interaction1(myPbdObjectA, myPbdObjectB);
  imstkNew<PbdObjectCollision> interaction(myPbdObjectA, myCollidingObjectB);
 
  - `RigidObjectCollision` can now be constructed with both RigidObject2+RigidObject2 or RigidObject2+CollidingObject
 
::

  imstkNew<RigidObjectCollision> interaction1(myRbdObjectA, myRbdObjectB);
  imstkNew<RigidObjectCollision> interaction(myRbdObjectA, myCollidingObjectB);
 
 - `OrientedBox` should be used in place of `Cube`.
 
::

  Vec3d center = Vec3d(0.0, 0.0, 0.0);
  Vec3d extents = Vec3d(0.5, 0.5, 0.5); // Does not have to be a cube
  imstkNew<OrientedBox> box(center, extents);
 
 - `RenderMaterial` now uses doubles, not floats.
 - Removed Octree collision in `SurfaceMeshToSurfaceMeshCD`. The octree is still present but pending touch ups.
 - `RigidBodyModel2` and corresponding classes moved from expiremental to main imstk namespace.
 - `DebugRenderGeometry` replaced with `DebugGeometryObject`. Usage as follows:
 
::

  imstkNew<DebugGeometryObject> debugGeometryObj;
  scene->addSceneObject(debugGeometryObj);
  
  // Can be called anytime, normally during runtime
  debugTriangles->appendVertex(Vec3d(0.0, 1.0, 5.0), Color::Red);
  debugTriangles->appendLine(p1, p2, Color::Green);
  debugTriangles->appendArrow(p1, p2, Color::Orange);
  debugTriangles->appendTriangle(p1, p2, p3, Color::Blue);
 
 - `DebugRenderDelegate`'s replaced with the already existing `RenderDelegate`'s.
 - Removed Vulkan, Pulse, apiUtilities, imgui
 - `NarrowPhaseCD` namespace functions moved and refactored into more general static intersection functions in `CollisionUtils`.
 - Name was removed from the light class, light names are managed by the Scene

::

  imstkNew<DirectionalLight> light;
  light1->setFocalPoint(Vec3d(-1.0, -1.0, -1.0));
  light1->setIntensity(1.0);
  scene->addLight("light", light);

 - Removed `Real`'s, use `double`'s instead.
 - `RigidBodyModel` now optionally built, being deprecated in place of `RigidBodyModel2` as it's more extensible.
 - `PBDModelConfig::m_defaultDt` removed, use `PBDModelConfig::m_dt` instead.
 - `PBDModelConfig::m_collisionIterations` removed. Iterations can be set per collision solver, given per interaction.
 
::

  auto pbdHandler = std::dynamic_pointer_cast<PBDCollisionHandling >(interaction->getHandlerA());
  pbdHandler->getCollisionSolver().setCollisionIterations(5);
 
 - `PbdModelConfig::enableBendConstraint` should be preferred when using bend constraints with varying strides. If `PbdModelConfig::enableConstraint(PbdConstraint::Type::Bend)` is used, stride will always be 1.
 - `PbdModel::initializeConstraints` functions removed. Replaced with extensible `PbdConstraintFunctor`.
 - `Texture` may also be constructed with an `ImageData`
 
::

  std::shared_ptr<ImageData> diffuseImage = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
  material->addTexture(std::make_shared<Texture>(diffuseImage, Texture::Type::Diffuse));
 
 - Enums removed in places were extensibility desired.
   - `Geometry::Type` removed. Use `Geometry::getTypeName()` instead.
   - `CollisionDetection::Type` removed. Use `CollisionDetectionAlgorithm::getTypeName()` instead.
   - `CollisionHandling::Type` removed. Use `CollisionHandling::getTypeName()` instead.

**Contributors**

Andrew Wilson
Harald Scheirich
Sreekanth Arikatla
Jianfeng Yen
Ye Han
Khalil Oumimoun
Aron Bray
Hong Li
Furkan Dinc
Ben Boeckel
Andinet Enquobahrie

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