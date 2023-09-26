========
Releases
========

# Announcement: iMSTK 7.0.0
On behalf of the iMSTK community, we are pleased to announce the release of iMSTK version 7.0. The Interactive Medical Simulation Toolkit (iMSTK) is an open-source toolkit that allows faster prototyping of surgical simulators and skill trainers. iMSTK features advanced high-performance libraries for physics simulation, haptics, advanced rendering/visualization, user hardware interfacing, geometric processing, collision detection, contact modeling, and numerical solvers.

Here is a comprehensive list of changes made for this release.

## Aaron Bray (4):
- ([f6c57d486](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f6c57d48648ac4679868e517a9cf2690f390bfb5>)) Remove explicit .NET version
- ([5562c4c3a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5562c4c3a51b91f1a1a38164db35430b6721527d>)) Remove SFML
- ([2caa94145](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2caa94145da3370abf3b66e3726ab5971c809f52>)) BUG: Add missing file to CMake
- ([01b2a6be5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/01b2a6be5a4f8f960b29dad97a1b22d4f65ae882>)) DOC: Add documentation to V&V classes


## Andrew Wilson (199):
- ([289177b3e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/289177b3e2452a03cf8c487e4159057d15d4758d>)) BUG: Adjust epsilon for PointwiseMap
- ([679f38203](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/679f38203ec5354238c515012f784f024ee7c35b>)) REFAC: Remove unused constraint names
- ([ddb0037e7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ddb0037e7aae24b126338b4d9bb504cbddb41597>)) ENH: Add vertex label render delegate and model
- ([039895ebc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/039895ebc6261655ea1ddf86e2476a751da29a19>)) BUG: Fix gravity in RbdModel
- ([879c580e7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/879c580e7386777faedafa89336da639aa32e797>)) BUG: Fix dt not being passed to PbdSolver, add public solve function
- ([8eeebcb4e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8eeebcb4e981a11e56de5e5f0418a3e9303987f4>)) REFAC: Use toTetGrid function in haptic grasping example
- ([ad60d5061](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ad60d5061cd03d08173da27696c941b8d96d4f5e>)) REFAC: PbdInjectionExample updates
- ([e9d8043b9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e9d8043b91e18339536d341125cdf79859d8c94a>)) REFAC: PbdThinTissueCut name fixes
- ([ff3281f9b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ff3281f9b5a4332b4aea9acb9701e2680292c6e3>)) DOC: Improve comments
- ([c2064ade1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c2064ade14bc08370072abb0f4f85287049ba64e>)) TEST: Add test case to fix
- ([9204c106d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9204c106d6b08e66457243f00714814facac27a6>)) DOC: Fix comment
- ([8e75f8f05](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8e75f8f0550eace0d9380706ce137fca577ac2c0>)) REFAC: LevelSetModel extend AbstractDynamicalModel instead
- ([5ec78ecbf](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5ec78ecbf4e6b3d30916ad5ad23c4fe6e192308d>)) DOC: RigidBodyModel references
- ([d320f5284](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d320f5284c074f74dcaa68a1e93f3f19c8451b39>)) REFAC: Default constructor for SphState
- ([c00a251d1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c00a251d189e64b35683d21569308a85e78bdb93>)) REFAC: Remove LevelSetState
- ([895ff69cd](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/895ff69cd4b9065deaf9eee1374308ac314528e0>)) REFAC: PointwiseMap getter
- ([cf87bd7bc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cf87bd7bc2287e21b145b324a27561f05761f4f7>)) REFAC: Unused forward declare
- ([0a1e73d84](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0a1e73d846e525e49ad1d93da219ad5a817a8b50>)) REFAC: Lambda instead of bind
- ([3a3d8a28d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3a3d8a28d23ba7eb81760dab2b84ad27f13ed338>)) REFAC: Ignore PbdConstraintContainer in wrapper
- ([32ac09e2e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/32ac09e2ed2fa7734631bf4f1459a0fba0e40647>)) BUG: Fix missing include
- ([4c43ca7f5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4c43ca7f51fc5addb84450bc05bfd5763159054e>)) REFAC: Avoid constructor usage on DeviceControl, templated DeviceControl for concrete DeviceClient access, DeviceControls stem off SceneObject
- ([87111ab62](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/87111ab62d33d28c5c4d21b47be96e8de4e1c711>)) REFAC: Remove old forward declares
- ([07e0ae1ef](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/07e0ae1effff6e34adbc98006b1408e31ca501b7>)) REFAC: Cleanup HapticDeviceClient
- ([141daae09](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/141daae09e7a3fd3844121e596a05796791ac2c6>)) REFAC: Remove unused forward declare
- ([8ec712bd5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8ec712bd579222f2f1529a97913da8b1f92ca26a>)) REFAC: Remove unused tracker out of date flag
- ([ae57a318c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ae57a318cc72503eb859534ad75daf95aca0c653>)) REFAC: Missing include
- ([cad3ab5b0](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cad3ab5b0b10eebda28a0e4d40031cc725e8ffb6>)) REFAC: Default name for colliding object
- ([0aa5ade27](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0aa5ade2797f9ec61682bf1b1582afe312693484>)) REFAC: Apply forces right after available
- ([4937c41b6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4937c41b62a575e3c4d6e73519f3f30dc3928b20>)) REFAC: Add LineMesh compute vertex neighbors & rename SurfaceMesh::computeVertexNeighbors
- ([c31ea2b5e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c31ea2b5e6ea54d3045c8635a012b438a0ed3fea>)) DOC: Improve some comments
- ([6fc719379](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6fc7193792293c9cbbfc84bf6587c85b9f1e3522>)) ENH: Add cell mesh
- ([4dc70115a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4dc70115ad072d04ab2ca9ad7e91e4e10a58f4e0>)) REFAC: Prefer getCells and getNumCells
- ([758e73646](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/758e7364644cc717101fb32af42d7446462d7c14>)) BUG: Fix mesh writing functions
- ([1b3628ef0](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1b3628ef0d992afef862348432f1b73f9eb0524d>)) REFAC: Wrapper updates for CellMesh
- ([9fcc2cc73](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9fcc2cc735f4fdf4654fdb59c6996ec586a849ba>)) REFAC: AbstractCellMesh in own file
- ([89a36bfe6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/89a36bfe6e99ff5b87b20b5897ddb9dbbe4feeca>)) BUG: Fix HexahedralMesh extra indices
- ([64cf0f98d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/64cf0f98dc8f7c546bf1467f2ff6cb99e54fc4d1>)) REFAC: Const ref in NeedlePbdCH
- ([89952fda4](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/89952fda4484f21c167433e59120bc0b12a9bde4>)) REFAC: Update getIndices to getAbstractCells
- ([93c08d02e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/93c08d02e6834ee076521f5cf01f9ce44e4c4a7d>)) REFAC: Update use of AbstractCellMesh
- ([c3cefc035](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c3cefc0352d92b158e600e250aa33557fff9a9fc>)) ENH: Ability to forward parameters to the functor for constant density
- ([cb838f539](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cb838f53963d6fbc8731fc11310611236670622e>)) BUG: DeviceClient should be updated in its own thread only
- ([48e09bf08](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/48e09bf080c0e3b338075581e128358db3dd2edf>)) BUG: HapticDeviceClient update fix
- ([fa7ce9053](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fa7ce905313e3455d02799105ff5c50f7168a243>)) Add option to disable screen text, for tests, on by default, fixes typo
- ([6bc1001f4](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6bc1001f4fc6a22a247948c99d751d8a7c2d4864>)) BUG: Fix Line dimensions
- ([d2ec0cb1d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d2ec0cb1d505554e528f97fab91c286b631190d5>)) ENH: Add option to turn off remainder divide for completely fixed substepping
- ([497ef9288](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/497ef92888dc48d4ead03ca1d70ce1f8f81bc253>)) TEST: Add PbdLineMeshToLineMeshCCD visual test
- ([c6f9f40e1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c6f9f40e13e86ab5f8e3d7bae73936cf17a85251>)) DOC: Fix comment
- ([742622371](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/742622371621b0dcc4b5a7cbc0f0cbd376d2654a>)) REFAC: Update csharp examples for control changes
- ([84b1bff57](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/84b1bff571ba821d161a42ddcdab93afd142faaf>)) ENH: Multi body Pbd and Rigid Body Pbd
- ([0572102b9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0572102b929fcd310712932081165e8195b8f903>)) REFAC: Example update for multi body and rigid body pbd
- ([370fb1a80](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/370fb1a8083d994e749f2eb676a29440318b3b7d>)) ENH: Refactor cutting filters and add LineMeshCut
- ([a5e87ce74](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a5e87ce74e00cf3355a61df8144cc4b7ac057abd>)) BUG: Add missing primitive case
- ([ed0debefb](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ed0debefb0e38dcaa09b2ce88405f998c62a0e48>)) BUG: Fix reset for external force and torque
- ([395bab47a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/395bab47a2ba73bf019f46769f205373cd7e21b3>)) REFAC: Use cwisemax in PbdFemTetConstraint
- ([957540e70](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/957540e701528c059a0eabe55354e7dc70dba0d2>)) ENH: Persistent virtual particle buffer and orientation update rotation limits
- ([9213f48a8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9213f48a83a37110519d63fa5a6a9d5c346d9160>)) ENH: Correct friction for pbd rbd
- ([38268c317](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/38268c3171d63c9b49be7200fc16bf320331ba08>)) ENH: Improvements to PbdBodyDistanceConstraint
- ([f14d883b7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f14d883b7403045ca8b783a71fae6205d4b87c4d>)) ENH: Updates to pbd rbd example
- ([5d381080f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5d381080f62f258403b86809add30d3af75b7d9f>)) REFAC: Remove SurfaceMeshToCapsuleTest
- ([ebf5768d3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ebf5768d3a964dcc1088cb3f1001ec67ddee5cc9>)) REFAC: Remame SurfaceMeshToCapsuleVisualTest to SurfaceMeshToCapsuleTest
- ([4fb7c042c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4fb7c042c0334ba801797a4d14d3d32afbf4c600>)) BUG: Fix FemTetConstraint
- ([46638dcd6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/46638dcd6a5c8e1dbc45f74791d08397c5c410f2>)) REFAC: PbdConstraint improvements
- ([fc3746991](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fc374699114466d231cfecf95965553c6fbba51f>)) DOC: Improve comments
- ([478abdd00](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/478abdd008796b71b9687e2bdac6bbf0d2ee125c>)) REFAC: Rename to cloth to tissue
- ([8696a8f6f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8696a8f6fe1e3756b79a5d54394aa1b50c73c112>)) REFAC: Rename collision cases, PbdCollisionHandling improvements
- ([15dda0c4e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/15dda0c4e1f0bef0776a8ba9d628fe8a79b11716>)) REFAC: PbdCollisionHandling Improvements, remove unused from NeedlePbdCH in dynamic suture
- ([dd70d9265](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dd70d9265873f320bbd0351ac7c77e6f78bf20a3>)) REFAC: Move resitution calc with friction
- ([af0ae117d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/af0ae117d28f09c1f0d3a9f8265295cb01801a8a>)) REFAC: Example fix
- ([51e8faba5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/51e8faba5e64810b4df3b528050f6f5e440b1b04>)) BUG: Avoid setting dynamical model twice
- ([ae920e5f8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ae920e5f8954bda955b3a1f7a7f1e330f09ff24a>)) REFAC: Update license headers
- ([361922ef3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/361922ef3f1370e651ec6286d0118f2489269663>)) REFAC: Remove unused PbdContactType
- ([970b37a51](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/970b37a515576cbc9bccdc515307d4c1a47b3831>)) BUG: gcc fixes
- ([4477c72ba](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4477c72baac6bf37a5a88ae0abefcdf851d1b17f>)) BUG: Revert FemTet Change
- ([ca5f98cbe](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ca5f98cbe85f2f35feb1e53543d76558505dcdc2>)) DOC: Improve comments
- ([8dfe06272](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8dfe062726734aaa845755e2d8af8c5fe023fadc>)) REFAC: Various fixes for pbd rbd
- ([d614da684](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d614da684d5a53cf6b05ec68fe78076060b7b7d8>)) REFAC: Replace epslions
- ([1857ca4dd](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1857ca4dd11c7f852b16c90574385f0c0540d7fa>)) BUG: Fix PbdBenchmark
- ([6439947c9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6439947c9c8125ae902068d3f216c55e8f4fa5f0>)) REFAC: Inline generalized inv mass function
- ([42ab4e2d8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/42ab4e2d85632e25ba27b78dbc3f61417644a8be>)) REFAC: Revert cloth example
- ([1b7f8a80c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1b7f8a80cf1c826ba8abbce60ca71ca3da7f98fc>)) BUG: Fix generalized inverse mass
- ([66174f5ec](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/66174f5ec686987f31206ac26aa90ea715967e1b>)) DOC: Improve comments
- ([bb3369357](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bb3369357c9a999a9504da7ccb1fbb3c3b9c4a02>)) Improve Comments
- ([722e054b6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/722e054b66b2ad38479a9827b485c09c84f37888>)) BUG: Fix relative velocity in friction
- ([b24f17ad6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b24f17ad63fc3420d1f113a6cacdcff2e809103a>)) ENH: Getter for handler constraints
- ([4356477aa](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4356477aa80b07044121150f0eba122e8c49d7ed>)) BUG: Text should be displayed by default, text update fixed
- ([5c777c529](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5c777c529aa047287bdc28cc768d4fbfbf2416bf>)) ENH: Update PbdTissueContactExample to use a two-way proper compliance interaction with new xpbd simualted rigid bodies
- ([408dc01df](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/408dc01dfbf111d2dadc0ea2beba8d0394366b17>)) DOC: Update license text
- ([10ec196b1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/10ec196b1678b060d1d6481cf9ad22f1de7dbc40>)) ENH: Get torque for contact constraints
- ([8dd1ea6e1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8dd1ea6e1f41ec3a584deca905cdb44ba2a66176>)) REFAC: Don't reset externl force and torque until end of update
- ([348ff2fdc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/348ff2fdc564916137304e6c4168731c5fe52d00>)) ENH: Update PbdTissueVolumeNeedleContact to only use pbd, other minor improvements
- ([3c1ade8fe](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3c1ade8fe0c28f52ee70fdd09c4d0089d44d61ef>)) DOC: Fix doxy comment HapticDeviceManager
- ([468b15f4e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/468b15f4e64711fceda08335f42556bdce7b7584>)) ENH: Haply integration
- ([dfa9a371a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dfa9a371a89ade01199e19be720a2bacad97a0f6>)) ENH: Haply examples test
- ([1923e56dc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1923e56dca060c6b217c929cd0cb64e7383bad06>)) ENH: Haply Hardware API support
- ([41654eb41](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/41654eb419460f2aa7670ca7a3f3e80df41fb226>)) REFAC: Remove BoneShavingExample
- ([f5fb827c0](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f5fb827c04f3255d22adf812756e5bcddc69a224>)) REFAC: HapticDeviceManager tuned to meters, Add DeviceManagerFactory, rescale examples, use shared_ptr
- ([19bd8c267](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/19bd8c2672d09c6749b0a923fe4cec3b42a684e6>)) REFAC: Unused includes in CDObjectFactory
- ([989727d60](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/989727d6088452c161e8483d8ee114466943de1a>)) REFAC: Remove HaplyTestExample
- ([f362e3d00](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f362e3d009f7982e7d682d5c1ef8790404531bb4>)) DOC: Add build instructions for Haply support
- ([79ead0ff6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/79ead0ff6041f13e00da1de25d089314022d1048>)) REFAC: Update data
- ([cf9780f26](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cf9780f260e51b39b2ae0d383c0cbd78435b373c>)) REFAC: Use relative data directory path
- ([032f5bb11](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/032f5bb11a954354001c2fbd59739b92df9290c5>)) PbdRbd Grasping Fixes, Fix PbdBodyToBodyDistanceConstraint, Other Cleanups
- ([00b8da4b6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/00b8da4b6b870bff726a4c62f76740f8d74a45cb>)) ENH: Fix wrapper for HaplyDevice
- ([1a3201e2e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1a3201e2e5bfea427bf8307d082437b0fc515362>)) DOC: Fix comment
- ([e10c3598e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e10c3598e6b4231e242f0e8d94ac47f8e1db8034>)) REFAC: Remove extra include
- ([5a8eab9ab](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5a8eab9ab970ed2eb4db8d77e27bbd7345a6390f>)) REFAC: Rename HapticDeviceManager/Client -> OpenHapticDeviceManager/Client
- ([91845fbf7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/91845fbf7bbd120ad831f5b2623134fed293514c>)) BUG: Fix VTKTextRenderDelegate issue
- ([3328a6dae](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3328a6daeca15f04270af12a26371a0613d800d2>)) REFAC: Pbd Test Fixes
- ([5ccf98bc5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5ccf98bc53a099b62d50090ac7c5be782762dc03>)) ENH: Cloning for Geometry
- ([48042e7c3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/48042e7c323958be102007829cbdf23b7269a3b8>)) ENH: Change enter/exit criteria in pbd volume needle contact
- ([c372d00a1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c372d00a1b194285992c7f54461d4b477e8e5d99>)) ENH: Update paths for laptool, add laptool body in RbdLapToolCollision example
- ([ddbacd6a4](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ddbacd6a402cae14f4185b870dc7349c9bd9eb80>)) DOC: Fix comments and other small bits
- ([e2767fd36](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e2767fd367c44a8633fa9fcb36f9d2f885fc29d1>)) REFAC: Remove some includes
- ([ceed901ea](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ceed901ea92932ca698bb4a109d2b1d3a1994634>)) ENH: Add base component model
- ([f7df3844e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f7df3844e2630d2935fe97efb811d440efd48df7>)) tmp commit
- ([d1e07b61b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d1e07b61b43e3a365810f2bf0fed7e262e981668>)) REFAC: Improve entity addition/removal, Rework the controls as components
- ([80b4334cf](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/80b4334cf5e2c31bc8518f63428cd0b368b34ae6>)) ENH: Rework initialization for dependent components
- ([0177bab7c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0177bab7c69c89e9bd4c9cfb9879b21d88929a1d>)) ENH: Update ECS, remove TextStatusManager, introduce FpsTxtCounter
- ([92bb3f5ad](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/92bb3f5adaafe4977b7be75b42ac2bfe501b9e1d>)) ENH: PerformanceGraph component
- ([40d12e9ea](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/40d12e9ea8eb5ce93bec846d19331e543df6e16e>)) ENH: AxesRenderDelegate and AxesModel components
- ([39f2c2ab5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/39f2c2ab5cfad4af8e602a4092a3a187bb970823>)) ENH: Needle and Puncturable component
- ([cbdc8bf69](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cbdc8bf695eb11bd5c9de3e896f64d2268e4a70d>)) REFAC: Move to Components directory
- ([0ba6e0c37](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0ba6e0c371681dba3b36e5107855d5d94f5cc6cc>)) ENH: Add setter for epsilon in cutting
- ([73a18a436](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/73a18a4368bde6c05ad058069bfe10f551d6254a>)) REFAC: Add controllers as components to objects they control
- ([edb82d29a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/edb82d29acaeea12c5ddb6e4fc90081682fec69c>)) ENH: ControllerForceText component
- ([c4240f41b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c4240f41b8dccda4ea5d6cbefe8c9c271877a021>)) BUG: Fix multiple needles puncturing same tissue
- ([1b6f3d30b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1b6f3d30b58e6656f85e3093d383625e69c33a7e>)) REFAC: DebugGeometryObject renamed to DebugGeoemtryModel, turned into a component
- ([c21298439](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c212984390561d5220c7761fb5c58c22e29c3e61>)) REFAC: Remove some creation methods
- ([780338570](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7803385709df03b36d38c43e938977cbf0398432>)) ENH: ComponentModel csharp wrappings
- ([19731d0cf](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/19731d0cf26a080569f5fa6a056d0b4ac13a7a42>)) DOC: Improve comments
- ([6b28d5357](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6b28d535764b55ec6e6213182167b12f995e377e>)) REFAC: Check nullptr Component in Entity
- ([a5b5636ad](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a5b5636ad18397007836c047412d7cd5f208c2dd>)) REFAC: Reuse addComponent call in Entity
- ([730fb80f5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/730fb80f5836b55cb36873788827c9c37b86e8fa>)) REFAC: Rename createDefaultSceneControl
- ([f8922b336](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f8922b336bc613ee2bdb089363c688c3b5f242fa>)) REFAC: Add weak_ptr
- ([d8a8cc95c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d8a8cc95c251fd863f11f6b3644b8130816762d8>)) REFAC: Rename Entity count member
- ([51201d37e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/51201d37edf631e620339ba1fae00f1b4e1b8fd4>)) BUG: Fix recursive component initialization
- ([d034a61eb](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d034a61eb94013c47b7c447afd4c737b7366802a>)) REFAC: Compentize RenderingDebugExample
- ([13cbebcaa](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/13cbebcaa926d69bfbae3711d77ec2f59f7128a6>)) REFAC: Move SceneControlText to its own file
- ([c46690ca3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c46690ca35bea7ceb465af1a804a333e2a78c39b>)) REFAC: parentId updates
- ([5b175e50a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5b175e50a09321378577472cd32893a7fc423efa>)) REFAC: Remove macro call in PbdObjectCutting
- ([9306a07ea](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9306a07ea5af77d2b6e2c322ff78130883bca456>)) REFAC: More checks in PerformanceGraph
- ([2c978b986](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2c978b986a816ed1e4e4ed594947c685bf201d24>)) REFAC: Improve Component documentation, add tests, add SceneBehaviour
- ([69858bb9b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/69858bb9b3b32f62c8c38ae8999879a076e4cbf9>)) REFAC: Rebase updates with PbdVolumeNeedleContact
- ([f92770d94](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f92770d94df3d84304ecaa990a70377f86b27dd2>)) REFAC: Remove unused function
- ([d16e28567](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d16e285677398089ca143d4d28a3f11f93aac758>)) REFAC: Add some null checks
- ([914366000](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9143660000ed46d23bd78ed900560ece62a3ae24>)) BUG: Fix issue with scene switching under new initialization
- ([d66937c3d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d66937c3d8fbe9de323b90edbed48e39a8a93b09>)) REFAC: Separate Needle into separate files and Needle module
- ([5c42f2878](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5c42f2878dde5ae3f504077c17915f8d710de842>)) BUG: Add missing includes
- ([4725421af](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4725421afb89ca30122e177705b76d7f9c8db5f4>)) BUG: Fix issue with data paths on linux
- ([c43c2c7c7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c43c2c7c7e638de881406e01626493df88ab0bea>)) BUG: Add ids and assertion bounds
- ([1167ac01b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1167ac01bd9625f711e8f0b2ade929614ec21bb3>)) ENH: Combine the collision and internal solve
- ([f485764c2](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f485764c24eee9b0cc969413869a9c865cc49d04>)) REFAC: Remove unused include, flip correct velocity statement, code format
- ([aa5756a75](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/aa5756a7513c92d1f3e35c3d2343ac2201e9b45f>)) ENH: Adds port hole interaction and body on body contact
- ([d6d10916f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d6d10916f65504bbdc16a3e05fd3474ddf88a626>)) ENH: Adds SphereToCapsuleCD and UnidirectionalPlaneToCapsuleCD
- ([fb094f0cc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fb094f0ccb7c5b3c9e52809fc9835619d1170b9c>)) DOC: Update comment and CD table
- ([1b9d9161b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1b9d9161b17edba6408ba7934382b0841042e9a3>)) TEST: Add constraint tests
- ([78b569355](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/78b5693552bb10f26e60e5cfd794f7e7ba614f88>)) BUG: Fix pbd test issue
- ([a18904aaf](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a18904aaf96b343ad5ff33869d3997ed95d5194c>)) ENH: Add haptic offset to exert forces off the center of mass, add parent id to cell picker
- ([babe02583](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/babe0258363bde28846b964a84b8d3cd705d83f0>)) REFAC: Fix sticky lap tools and improve some comments
- ([0c5c8a42b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0c5c8a42bc6e1add5a17f5ad1663c369352090a7>)) REFAC: Tighten up the thread in the tissue in suturing example
- ([cef608671](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cef6086711400240480d22b47d08e2c6241dda44>)) REFAC: Better warning message for PbdConstraintFunctors
- ([4a9055122](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4a9055122f0d22628c4ef5e8bee401f6140a2dd2>)) ENH: Add pbd rigid grasping and relevant examples
- ([2301d1a3c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2301d1a3c9d2b9c7bfcdd84954be89e0263a30ac>)) BUG: Fix issue introduce in CellPicker when fixing cell ids
- ([fe011c8e5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fe011c8e57daa3ee7201259e028d944e36a4270d>)) REFAC: Mouse controls and rename for rigid in deformable grasping example
- ([fc633b3bb](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fc633b3bb85242ea2fc2a19319f9804583a26bba>)) REFAC: Move example to pbd group, include remove
- ([0f77be8db](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0f77be8db557c9b7ef23b7fbb09c286f4aeca58c>)) TEST: Add grasping tests
- ([3825e3686](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3825e3686f65c2e1e224891948b84871ab07e307>)) REFAC: Improve comments and renames
- ([24be036df](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/24be036df1b7362545f0cc8746614056fa1fb1e2>)) TEST: Add CellPicker tests
- ([64ab7ab3e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/64ab7ab3eb3893b86a7428aa2ffab61129902a83>)) REFAC: Fix HingeConstraint rename
- ([dffa44a0a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dffa44a0af4ba46bff1c7afc635c74022febef61>)) BUG: Fix some linux warnings
- ([fade41a3c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fade41a3c74785f2125f926a6f77cfb4f32effe8>)) BUG: Ensure order of cells by CellPicker
- ([63381b517](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/63381b51759fd0d690b38ff28cecd831f5432383>)) REFAC: Fix FemurCut bug, remove unused virtuals, offset in IsometricMap
- ([f34bcbd58](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f34bcbd58cf7dadbe8ef7799cf60f7d41ac9f8b2>)) ENH: Add TaskBehaviour for task graph based components
- ([759601a77](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/759601a773de9b7c14429ba993c40fec40b3aa05>)) REFAC: PortHoleInteraction use TaskBehaviour
- ([3f6d49ba1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3f6d49ba105a4bd4b31570579c98dea06f0d170c>)) ENH: Update Haply to 0.0.5 and add handle support
- ([e9307e129](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e9307e129287e2665e8a425b6f63359346659ced>)) REFAC: TaskBehaviour from Behaviour, add test
- ([2995fe5c3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2995fe5c3e9fc090b9ccfa0a69adbd734950f876>)) REFAC: Update doc and rename to local offset
- ([a30d71b17](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a30d71b17dee6e5a6fd1c0a1db69460a2415bba2>)) BUG: Fix issue with map setup when using differing visual and physics maps, add test
- ([352904b45](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/352904b45920eb7f67139c73f1d69a7732841ea2>)) BUG: Update wrapper for ECS changes
- ([e3209fdc7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e3209fdc7b4271a594aeb1d16d01cc359906a6d6>)) BUG: Add check for msvc version when using Haply 0.0.5
- ([00a9e1d67](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/00a9e1d67f9000f802b34a2889e0c94a1f10a9e5>)) BUG: Fix VR controls so multi buttons can be pressed at once, update camera movement, fix release on grasping, and move FPS counting down to imstkViewer base
- ([cd32e3c03](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cd32e3c0389bb0cc3999e0406c1f1f2ff7e87aa4>)) REFAC: TaskBehaviour in Behaviour
- ([00884e977](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/00884e977b3e1d8a8ecd16720822ec05722398d2>)) REFAC: Remove imstkNew from LevelSetExample
- ([b408005e3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b408005e306aac788c533e9d73f30f69510aedeb>)) REFAC: Add default construction for LevelSetModels config
- ([9424dedf9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9424dedf9373422b55ad0be69fcbf6eb3de76464>)) REFAC: Add initialization for image bounds
- ([7ef326ca8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7ef326ca8db8a4f3189fc2689e6b477579575907>)) BUG: Fix issue with parallelization in SurfaceMeshDistanceTransform
- ([fde117e89](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fde117e89a5bf5a4ac0308b2e900488fe09d61b6>)) REFAC: VRCameraControl comments, remove test code
- ([76aae16cd](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/76aae16cd9508064cbc3f738662e633b337741ac>)) DOC: Add Behaviour documentation
- ([70dda2e31](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/70dda2e31fa495729775325c065e066c52ee3d2a>)) BUG: Fix port hole conflict from merge
- ([a1a9b3a3d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a1a9b3a3d0f764fbb1b6f0a34414262a4886797a>)) BUG: Fix port holes in haptic examples
- ([498512db9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/498512db95e682ae02bdf746183d9a8cf4569c45>)) REFAC: Enable VRPN in dashboard
- ([3e37b9631](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3e37b963180d9cd68c3e1395744d3c5f38c787ba>)) BUG: Fix an issue with observers in the EventObject
- ([e096c99c3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e096c99c3aa79f71ce4bccc4d0b3e40cdbcea9e7>)) REFAC: Fix some warnings
- ([b4eaf5a07](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b4eaf5a077bfd51cfaebb74bad4793fd35b68899>)) BUG: Update EventObject to deal with deconstruction
- ([06ad3d2c7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/06ad3d2c78b94f04eea31922f08e57fb421d0e03>)) REFAC: Fix connect calls to use shared_from_this
- ([66e8e77c7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/66e8e77c7df1a180da94296c2803125f1a6b817d>)) TEST: Add small test for the viewer
- ([a93cb3bbf](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a93cb3bbf06f802c69253c15a2cbe729018fdafe>)) TEST: Add some tests for adding/removing scene objects to viewer+renderer
- ([27afd6bf8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/27afd6bf86a2f08882d5cae5c1b710e4ac464986>)) REFAC: Better collision stiffness and compliance defaults
- ([a6e575e68](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a6e575e6880cab16c777943ca2bba2adc10da5cf>)) DOC: Update documentation


## Ben Boeckel (8):

- ([c2706971c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c2706971c0bc8bf825feb0ede040ca89a6ec2f00>)) imstkGraph: include `<cstdlib>` for `size_t`
- ([b2e28f782](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b2e28f78223e9e40da927488b97c046752a72b07>)) VegaFEM: update to fix missing includes
- ([e553ec077](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e553ec077bb2657083990f327ad9e016d2c0a111>)) tbb: update to 2021.5.0
- ([12a1141ec](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/12a1141ec1f594539f79a3592f8832937c9f90ce>)) tbb: disable `-Werror`
- ([fba361c9f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fba361c9fbb816af2991ccfcf0dca0e83381b0e3>)) gtest, vtk: force install into `lib`
- ([533314e92](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/533314e9295bba7be691791b977d53a56c332667>)) ci: add Dockerfile for Fedora 36 testing
- ([1706f422f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1706f422fd3a434c0b61a8403d3b810288238b77>)) gitlab-ci: add basic Linux job
- ([4d4d78605](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4d4d786059b2ca76ea203d6ea338942b526723fa>)) gitlab-ci: use arch-specific tags for OS selection

## Connor Bowley (1):
- ([533efe701](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/533efe701d5a6233e4467c205b56ea07d0eabd96>)) BUG: Fix X Error during shutdown on Linux

## Harald Scheirich (64):
- ([32f367e3e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/32f367e3ed416a42ab922de53e91babaa6a80779>)) DOC: Update Openhaptics links
- ([38722a164](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/38722a1643bcf6a004341ef076f7ef0487904e1a>)) ENH: Add at() function to vectors
- ([dbef83915](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dbef8391594926826804954c97e4d9349c727c20>)) ENH: Add polymorphic `clone()`
- ([5819273cb](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5819273cbf7eff445d20037c1f3444c9d8c9ab88>)) DOC: Change license header
- ([016268cfb](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/016268cfb496ee92bba3560e9004181e892d0755>)) REFAC: Fixup MR
- ([8891fc2c8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8891fc2c82902fd0fb609c444441a9ab657908d9>)) BUG: Fixes issues with wrapped classes in the device hierarchy
- ([62bcc6dc5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/62bcc6dc5694d1655f7007b519e06d330ac232e6>)) COMP: Remove unused configuration file
- ([d7d8f8b7f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d7d8f8b7ffb5d85fe21636265979f2b929d29bbf>)) COMP: Renaming external project broke CI
- ([51e409518](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/51e4095186cb2178ac2d6ed8589ef30da9c63823>)) COMP: Force C# project to be configured more aggressively
- ([575839e7d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/575839e7d2240b1701a8963c9f5b25a04b6963e4>)) ENH: add accessors for rigid body position and orientation
- ([e4aa1ab9c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e4aa1ab9c3390bc8df3d99152a3999895f819fc1>)) ENH: Wrap PbdObjectController
- ([a74545ac0](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a74545ac037509778070dd09c8c70d4ce10f7ac9>)) ENH: Enable a fractional density for the connective tissue generation
- ([2b86a9de6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2b86a9de695bec18a4be036b9b13432d8f0ecb6f>)) ENH: Remove constraint if it already exists in `enableConstraint()`
- ([f567db13b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f567db13bb503ed63ff07d0cc270b1dc497db082>)) BUG: Uncrustify
- ([6f5c7748b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6f5c7748b02b1efac730e53adb469b1c5d7d5f4b>)) ENH: Improve distribution of the endpoints
- ([00b6c760f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/00b6c760f8fd463db0075894c90a64fd08979ce5>)) COMP: Remove extraneous include
- ([9270931a1](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9270931a12914947fe71faf6f96f063ee1789b9d>)) BUG: Fix parameters for copy operation
- ([210cfe24c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/210cfe24cde65b6a7c8c0b84476bd87144ff0b60>)) BUG: fix a range error
- ([60a866f99](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/60a866f99f3f5dd033dea2167f3e46f7e9fa3f82>)) ENH: Expose CD Factory
- ([de1ccd3da](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/de1ccd3daf7fc89700415aa13391d3a9a00f00e1>)) ENH: Add constructor with fewer type restrictions
- ([ca22d78c8](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ca22d78c8c74b949b71507df63f1ef83b53acd51>)) COMP: Fix linux build
- ([dae8e5ef3](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dae8e5ef331c4cf57beec6cc9191184b77c2949d>)) BUG: swap members if necessary
- ([bd4191a27](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bd4191a27cc68823e0376a607cbe5a70b2a047e2>)) ENH: Show average physics frame times in stats
- ([5df176876](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5df1768764f4e50ffd3e460500006179d167e5e5>)) ENH: Add unit test for closest point on segment
- ([cf5708e08](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cf5708e082f1d07141254f26eea13dc016937cff>)) BUG: Add test and fix calculation bug
- ([e886951b7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e886951b72eb84d733e562f92be2c879c90bf380>)) ENH: expose distance constraint
- ([71db5f439](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/71db5f4397188ae7885cd6b3fa5ea1f708dc6251>)) ENH: Expose ConstraintContainer and support
- ([557bcb6c7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/557bcb6c765aaa5fc802988406e1e06c99abef35>)) FIX: Order agnostic constructor was not working correctly removing it
- ([f2aae63d4](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f2aae63d40df009227e358ad8ae629b1fbb05837>)) BUG: Fix Fluids example
- ([cac1e7ca9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cac1e7ca9d15a4379fa396354cd901250d6a767f>)) PERF: Reuse Collision Constraints
- ([68a63b31e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/68a63b31ee7eae837e916729ce2fc10f594a05e2>)) BUG: Fix Tetrhedral volume calculation
- ([ff58c5174](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ff58c5174ef1efda0423bb89d802de9142090dd0>)) BUG: Fix deletion of cached constraints
- ([1b49b274c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1b49b274cf8433c7e2b90540d9eb3d2626f5fe01>)) BUG: Invert the rendered forces to match the controllers inversion parameters
- ([9b8ec3c01](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9b8ec3c0175e2ebbb455e2db22d50a0826ebe109>)) ENH: Expose task timings in swig
- ([909e2508c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/909e2508c5c5aca9f628ea30f0924c77f0a458ad>)) ENH: Add accessor for single body in model
- ([6e61d167e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6e61d167e90b5b66b488294d034b4eb2a693de6e>)) PERF: Remove extraneous geometry updates in grasping
- ([5d7073d17](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5d7073d177accd1191ab4f51c674b695d559a3e7>)) ENH: Allow configuring the strain model via cell attributes
- ([b53cc30f5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b53cc30f5ec8db1e9c500eb3740f940551db660c>)) ENH: Allow writing of obj files
- ([59197bb69](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/59197bb69705b4f42873c2a163286b1c94ac6fc2>)) BUG: fix issue with strain parameter setter
- ([80722b7f9](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/80722b7f94064b1f45b80b224ffeaa7ebe8230d4>)) ENH: Add accessor for cells of a vertex
- ([9e159e152](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9e159e152ccac01552e59a7347678f950cb612fb>)) DOC: Fix comment
- ([2f166de9b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2f166de9b51dd2795ad66aa775bac08ad56b7865>)) COMP: Fix test build (was missing include)
- ([d7d03000a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d7d03000a9a4bb63078e652e5aca6fa91e2c3b65>)) BUG: C# CastTo was causing an exception when passed a nullptr
- ([2018d1c90](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2018d1c90136cbd1dc637c246b2c4a2105989755>)) REFAC: Remove RigidObject2 in favor of Pbd
- ([a19410bdd](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a19410bdd03fdf005d81d7167a4ffc4b9396951f>)) WIP: Move puncture detection into separate function
- ([43303168f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/43303168fa7709c820423b7eef3e4ec9a82f4c3b>)) REFAC: Move stiching support into needle library
- ([ecbbed67b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ecbbed67b426c75de4785f5d1b55d14e8aa1368e>)) ENH: Add Suturing support to C# Wrapping
- ([4eb93c520](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4eb93c5205dbf43f8f2df1ac45a7804b379d2d3a>)) WIP: Move Needle To Thread maintainance into Collision handler
- ([42ac66f14](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/42ac66f14872b067f5c680d3c3ca4f0fa3828068>)) REFAC: move Needle interaction into imstk namespace
- ([f1088c337](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f1088c337f587fe9f3f7d58e1307064db9df7734>)) BUG: Fix Needle detection on exit
- ([3ac552584](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3ac55258489fa6bcf07be2c12b2aad452452ff17>)) BUG: fix wrapping for puncture data
- ([fbbc59b12](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fbbc59b12850600109a9d7d71659ec95f27db2f1>)) BUG: Prevent the constraints on the thread from being flipped
- ([6930746db](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6930746db269a34b425477bf2f066db4097d6d3e>)) Uncrustify
- ([15783dc1c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/15783dc1cb2a5b0c6ddf9f223d5bf493246032b4>)) ENH: Prevent Haptics manager from posting events
- ([a269bf023](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a269bf023fdeac07c0c04899fa6bdfa0f1ec80fd>)) ENH: Add test example for two devices
- ([cdf076724](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cdf0767241c7d691dbf0474c2c97ad4f26ff6e06>)) ENH: Refactor OH Device client and manager
- ([c04abfa08](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c04abfa089b982a73c9e415f054089a55b72f8bb>)) BUG: Generate strain data on get
- ([3148d32dc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3148d32dc8767b07b10673b64eaa31787e03029c>)) BUG: Fix new devicemanager shutdown
- ([fa8b73c08](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fa8b73c08612b9b10eb35647c19077fcc042dd62>)) BUG: Fix Strain Test
- ([f6c8fc357](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f6c8fc3571d6877070d1e7803a52e9e430a0179e>)) COMP: Remove unused audio example
- ([6557fa592](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6557fa592791f5e7111f733daf46daea8a952598>)) COMP: Disable PBDInjection example
- ([041399126](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0413991262eb894a5e5d61baacd79cbc56ecb5fe>)) COMP: Fix compilation warnings
- ([edcd9c90d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/edcd9c90d866d6dd741dea88a84fbe1a0e35bbaa>)) DOC: Stub page for ReadTheDocs
- ([c15c6530a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c15c6530aa9cc62bf0b35a12cd0cba07bd9f9e1e>)) DOC: Improve Clarity and spelling

## Jacob Moore (40):
- ([77f34f283](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/77f34f2835b4d5c1ecaf4bdb4f4f65454a78a7af>)) ENH: Handling for Tet Inversion
- ([3a27f906d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3a27f906d4b7a1d90f75ceb9d05a47be9675dce7>)) ENH: Adding screen control info
- ([e26a4a16a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e26a4a16af7fabf1bf1bcf231d82334603e4edba>)) ENH: Adding missing cases for capsule to surface mesh CD.
- ([9c135d819](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9c135d819934b40ae470ef7c5b3338c2e0498f00>)) ENH: Adding default options for generating collisions
- ([479d70b16](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/479d70b160369d0ea1f969fa6dad87b5eee3f3cd>)) Update header imstkCDObjectFactoryTest.cpp
- ([6fe43172e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6fe43172e6820f183b2eaa0d10aa235f8b73ce2f>)) ENH: Improving stability of collision constraints near a boundary condition.
- ([06acc9ba2](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/06acc9ba281ea8916d79365f950913a2de309d09>)) BUG: Updating analytic geometry data in initializer
- ([288893e65](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/288893e6589ebc26663814be3c29db1a28e4940d>)) ENH: Adding LineMeshToSphereCD and LineMeshToCapsuleCD
- ([c7b895a53](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c7b895a53503464775ae16c96b5c7cdde761b68e>)) DOC: Improve documentation for EdgeEdgeCCDState class
- ([66f1af16a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/66f1af16a58de0fc12cf9028c16d0a4e2cdb2ac8>)) Feature/apply to data mesh normals
- ([7a2e6963f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7a2e6963f5626edb030556507a143a3130ee535b>)) BUG: Velocity compute and scale fix
- ([25140a27a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/25140a27a495821c1de251e5803089c8ad460bbc>)) Feature/fix append mesh
- ([6596cbdb7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6596cbdb7eb2c6f70fc316c6c545180608a7abb6>)) ENH: Strain energy models
- ([15463685f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/15463685fc95846ded679a2f7e811247b60bf452>)) Feature/ProximitySurfaceSelector
- ([da1d4073e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/da1d4073ecc5249dff3c834938211d2505c71cc6>)) Feature/barycentric coordinates surf mesh
- ([80461db26](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/80461db26c6e538ac4f69fcb0a95a988c2abc260>)) ENH: Connective tissue generator
- ([b16c7e995](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b16c7e99547d06a31459c2a888b574dd0956ad08>)) ENH: Connective tissue constraint generator
- ([071014bd0](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/071014bd0692d72890838a6709e99e2ec15e4412>)) Update Maintainance.md to include SWIG wrapping
- ([f2147107e](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f2147107e939c6a800eaa8780deb4322bf9fc1ee>)) ENH: pbd object cell removal
- ([629f99343](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/629f99343deb03427750812bfc1f569d06f2577d>)) ENH: Adding Tearable component.
- ([69a9e0e77](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/69a9e0e771a60de7127741d0ebb4f2f92c7aca3b>)) Feature/burning dissection
- ([7f35a1dd2](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7f35a1dd2fd62e05a70fcb4c5060a3a55a70f07b>)) BUG: Debugging collision
- ([016639d9c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/016639d9c19995a1583e4532b56aac8c068c6444>)) ENH: Fixing ct wall constraints
- ([c62269188](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c6226918822af22fe58854f1feb3437a2bd60e79>)) BIG: Fix collision order
- ([05c9e1f6d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/05c9e1f6dc7e817c837cf9d2742f39a88c164ab8>)) Feature/adding closed surface capsule collision
- ([8f13fddb4](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8f13fddb42b7fb9f5c7267bb69662869b12af561>)) ENH: Adding 2 way interaction WIP
- ([7641c9530](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7641c953076a9fa192cf6b8181712983771cfd5d>)) BUG: Fixing thread penetration WIP
- ([7b832dc69](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7b832dc69f6b6b91a2f80f7825d5d851461820ca>)) ENH: Adding setters and getters for stiffnesses
- ([c5343a7f7](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c5343a7f7eab21b14aaa4f566b4e49bab7d3b3c1>)) ENH: Fixing repeatablility issues. Fixed issue where needle could puncture on side, multiple clean ups
- ([62106e7e6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/62106e7e6471ac2e5f354a6ffc08d1efbb85ae43>)) ENH: Exposing threshold
- ([bfd43ec9d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bfd43ec9dac7cdfaf45020ff74c72d5116154ee8>)) ENH: Adding 2 way constraints for needle to thread
- ([358b39b3c](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/358b39b3ccb54c2c216f6b68eda35c71449b8bbf>)) ENH: Adding stride to suturing for performance
- ([5d2b6a26a](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5d2b6a26a5f5b5a5ceae05c67e8760cd453b49bf>)) ENH: Adding ability to stitch multiple times with relesing the thread
- ([131b187d5](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/131b187d50773bb49feeb3b54fbcf4bf5d809f9c>)) ENH: Adding getters for puncture data
- ([5b99e1a06](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5b99e1a069b132a039ff1a8b13b82cd8a3f044ed>)) ENH: Adding wrapping, and allowing BC constraints on setup
- ([655b0ccc2](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/655b0ccc2cf43f29e58c921b1ebe1f24a73bca9b>)) ENH: Adding function to get world coordinates from cellID and barycentric point
- ([2333a994b](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2333a994baadc782be03bdfc4cc65a89840a87f6>)) BUG: Adding debugging and fix crash
- ([9e7848948](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9e7848948191bd9d67547a4654863383e9a0c670>)) EXAMPLE: Tweaking 2 device example WIP
- ([9fc285531](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9fc285531c888f28a2b2abb56a3d5c16b0adb23d>)) EXAMPLE: Adding stomach to 2 device example
- ([50bf2cecc](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/50bf2ceccf55d12c5b960acdea20a8f625862910>)) EXAMPLE: Updating 2 devices

## JustinMWoo (1):
- ([03652c3b4](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/03652c3b4f7a613dd613e03eb035d62215ac223b>)) Added getter for ModifiedChunks

## Stephen Crowell (4):
- ([b520ea992](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b520ea9926906df54cf108852f7cfcd5376a7123>)) ENH: adding type name to Constraints
- ([72a77929d](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/72a77929d74c226e41b007ef9f6ed2bb8277f9b1>)) ENH: add VandV
- ([8e3b20c58](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8e3b20c58d499f2900e68a23fcb891756ce8e1fd>)) BUG: fix CMBBE tests
- ([89a3b4df2](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/89a3b4df21fc531502dcacb3fc5d5db9faf8b5c6>)) BUG: fix broken capsule to cube scenes

## Tom Birdsong (2):
- ([857e658b6](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/857e658b6f500875de02b64ae600a739a2173b62>)) DOC: Add steps to reproduce Linux CI build in local container
- ([846e8a88f](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/846e8a88fbc15187aef4f6b4290a6965b21018f3>)) Fixup for !917


**Contributors**: Aaron Bray, Andrew Wilson, Ben Boeckel, Connor Bowley, Harald Scheirich, Jacob Moore, Stephen Crowell, Tom Birdsong, Shreeraj Jadhav (unintegrated ECS Branch)

Release 6.0.0
-------------

**Announcement: iMSTK 6.0.0**

On behalf of the iMSTK community, we are pleased to announce the release of iMSTK version 6.0. The Interactive Medical Simulation Toolkit (iMSTK) is an open-source toolkit that allows faster prototyping of surgical simulators and skill trainers. iMSTK features advanced high-performance libraries for physics simulation, haptics, advanced rendering/visualization, user hardware interfacing, geometric processing, collision detection, contact modeling, and numerical solvers.

Here is a comprehensive list of changes made for this release.

**Andrew Wilson**

**Enhancements**

- ([2e7c3b1b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2e7c3b1b104a4673394fd81b8f2285b62b2ad85b)) REFAC: Connect & actions implemented in SWIG
- ([0f5a0f44](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0f5a0f44e1c7f6d221bcd8913089a829b964c871)) ENH: Ghost tool for static suturing example
- ([70b922fb](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/70b922fb45f4199bb9c87824b751e8f260fb4432)) REFAC: Default to adaptive stepping in SimulationManager
- ([d5995889](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d5995889629ddbed81c4a176a545f05625d15532)) ENH: Mouse controls for FemurCut example
- ([75d41d96](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/75d41d96afcbeef75681b01bacf51252aa4f4e84)) REFAC: PbdFunctors now created in the config with polymorphic interface
- ([2ab250d1](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2ab250d172340f9f2eaca4404567727c60926803)) ENH: Cylinder mesh generation
- ([8f710ace](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8f710ace59403a54b517f7a045b70b1e51438fdc)) ENH: Cloth grab example
- ([8790bb94](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8790bb941ae8ee9b959a11c094dd93a2c2ff8956)) REFAC: TrackingDeviceController Rotational Inversions
- ([0ae58230](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0ae582302f815d10ce0cb15b99178ce60cc11690)) ENH: Assimp reader can now read LineMesh
- ([d22d38f5](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d22d38f588215bec24f3ee6ea5c2fe6dd12f9976)) ENH: Interactions now SceneObjects, CollisionGraph Removed
- ([8361c0ad](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8361c0ad19949ea936c745805cae3cb82d00ad94)) REFAC: begin and end pick can be called from the interaction, getter for the node
- ([4b043ff7](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4b043ff71143b0e89ff1c273d6938d09d26912e2)) REFAC: Change PbdPicking to occur after internal solve
- ([7deab9a9](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7deab9a94ce3156cc964832bd6df5638c9020478)) ENH: Adds vsync toggle
- ([e89f1d3d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e89f1d3d0fc25864392e0cf6178d07cfc7c4aa81)) ENH: Upgrade to VTK 9.1
- ([e491885d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e491885d27618e06c039e714ad546f5db7afbbab)) ENH: Adds clearcoat model to iMSTK & colon fly through example rendered with clearcoat
- ([7c890690](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7c890690f1fd14904ab0e3bbd31fbfac9d5ff74f)) ENH: OpenVR support for thumbsticks, triggers, & grips
- ([e964e167](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e964e16710c3a983a8d65f6dbf909d05f504e29f)) REFAC: VR camera fixes, scene camera now pre multiplied to allow VR camera controls
- ([bb947658](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bb947658ff3fe9eb4d585aa469060636fcca4203)) ENH: VR movement controls implemented
- ([2848b07f](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2848b07fb4db9ef0aac1d19ecea0aa55db87cc94)) REFAC: Texture wrap type enum & border coloring
- ([ff2aedcf](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ff2aedcf054ae76c47976872f5820737c7b03a2a)) ENH: Upgrade to TBB 2021.1.1
- ([5c6b0c68](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5c6b0c681e0ae9eb4038cd2f257dc44ad89eb004)) REFAC: Split libraries off creating Materials, RenderingCore, RenderingVTK, ViewerCore, and ViewerVTK
- ([ad7e52cc](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ad7e52cc07c7736668fd4068a0c527af22aff677)) ENH: Adds mouse controls for static suture example
- ([0a3e5e83](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0a3e5e83cb7f2dbe055c56d2a673e0a5ad9cee33)) ENH: Adds critical damping to RigidObjectController
- ([a2dbaf81](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a2dbaf813664343e9b06d74012cd9aabf5d9f05e)) REFAC: Adds dynamic flag in RenderMaterial avoiding GPU vertex buffer upload & CPU normal recomputation for rigid & static meshes
- ([8dcf84b8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8dcf84b8d902f2cf481288f19e860c1b0a87411a)) REFAC: Remove PhysX
- ([a99c1d9b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a99c1d9b831c4cc899963f8e9fae311f8ee41553)) ENH: Introduced delegate hints & RenderDelegateFactory
- ([f769b590](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f769b590d9146bdae83b07e33e5eef501da78d73)) ENH: Adds ghost tool for virtual coupling example
- ([9c78db5c](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9c78db5c2551a923b0814be502abc7e56dccbcd2)) REFAC: Remove parallel runtime for task graph
- ([865a629d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/865a629dae0395e9cd35ec6b48e4d70f14f6bdf8)) REFAC: Add task node global ids
- ([b5ae8f71](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b5ae8f7177a95f42b036aa0624dbdc6a13a6237e)) REFAC: Add default SceneObject name, unnamed
- ([47b66ea7](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/47b66ea7e315cd9a8c57c0c05e73a0a41cb4a614)) ENH: Refactor PbdPicking to remove collision handler & add element picking
- ([3f3e6110](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3f3e61103b186f082f8c34f7cb9cd274d35a913c)) ENH: Generalized picking introduced
- ([6bb0ee1e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6bb0ee1e9c4003bd72d0be9ba7fce6bab1493ae1)) ENH: Update point picker for tetrahedral grasping
- ([4e4136f0](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4e4136f05d005676e29fca1b3f7a065f5392044e)) REFAC: Make laprascopic angle movement relative to time
- ([ec64d6dc](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ec64d6dcefc5ad60fbbaf55be7d5e0b96e8dd40e)) ENH: Ray cast point picker can sample first hit or N points
- ([ddceb571](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ddceb571a16094c757ed0978d6056cf00e5456a3)) ENH: Support vertex data in cell grasper, such as with PointSetToSphereCD
- ([c70539d0](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c70539d0b3bef6c6d7cfa95d5a90cbdec74e9edc)) ENH: OneToOneMap support for PbdObjectGrasping
- ([1d1dba48](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1d1dba48b5817ab4062c112b2181f95dcc3bbb03)) ENH: Add PbdObjectStitching
- ([aff4c089](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/aff4c08958f9855363a7f47af16ef0cdb2f5596f)) REFAC: GeometryMapper to GeometryAlgorithm refactor
- ([f0c655a1](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f0c655a19f8793dac8935556ef8255209814daf1)) ENH: SurfaceToTetraMap added
- ([e6d8ebd2](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e6d8ebd23254eb6f27d83381d10693cae01f81dd)) ENH: Stitching support for thin tissues
- ([7dd6e8d6](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/7dd6e8d69053dfd4ef7681339b996dd0d8f47be2)) PERF: TriangleToTetMap now in O(n+m) instead of O(n*m)
- ([4bc001ae](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4bc001ae697adc8c74617baf3fb3e8ad39a0d943)) REFAC: Fix dependencies. libusb, libnifalcon, & ftdxx not required when not building with VRPN
- ([c6634178](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c66341789b43ff3d5b5f4205ff2a6de4a818276a)) REFAC: Remove globs from all imstk libraries
- ([2f14dd4b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2f14dd4bbcf9df5a3e0a44d07630f6c08340766a)) REFAC: Remove libnifalcon & ftd2xx
- ([e37e47c2](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e37e47c2fba0f73eba3a8d060a7abf8e8b904f3a)) ENH: Adds build option to build without rendering
- ([1a27dfc4](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1a27dfc44947423cb1ad8f6fa7c3e13229b7d8ce)) REFAC: Introduce VisualTesting base & reorganize test library linkage
- ([874395a4](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/874395a4fc38d4f764c6bc4e3979a9f9a3abe532)) REFAC: Add line, triangle, & tet grid geometry generation functions
- ([f1374ec5](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f1374ec57dfb22824f046de17e76b43dea1b27c5)) REFAC: Default no friction for pbd collision
- ([41219474](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/412194744264322d9e53dac3c0280b7072481f1a)) ENH: SceneObject names optional, function to produce unique name
- ([9f998fdd](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9f998fdda9ae412785dbf554198f76960baa9add)) ENH: Adds .vtk file writing for SurfaceMesh & LineMesh
- ([2fcd924e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2fcd924e4f0bc4ea316acfdcef0316e1d245710b)) ENH: Adds binary .msh reader, refactor ascii .msh reader
- ([3c59594b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3c59594b06e3a1c3febec2feec2fcc9838d4d710)) ENH: Capsule To Capsule Collision Detection and example
- ([dbd47504](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dbd475044c9fd97dab3dc4d1d3c82c6420c3ffb7)) ENH: Default to 0 angle bend constraints, provide parameter in enableBendConstraints
- ([3e3551bf](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3e3551bfb2b54fb1770c9819671d35528400165f)) ENH: Add haptic device support to PbdSutureExample

**Testing**

- ([97c44f84](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/97c44f847ca45f06c85ff1e0f7352743cab03acc)) TEST: Add test for initial camera looking down z
- ([46735548](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/46735548358974160f51fc6dccaf93277937e2e7)) TEST: RenderDelegateMock test
- ([dbd8779b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dbd8779bbaa22c6fdd92b65ebb728db1cf70a009)) TEST: Add point picker tests
- ([657c28a6](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/657c28a6464b6c2e38181cf0812ea1a85ff8a807)) TEST: Max distance boundary value test for PointPicker
- ([e03fa0b0](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e03fa0b034ed24b701e7cab102b34eb32e0f4a35)) TEST: Barycentric and ray function tests
- ([ad96cf48](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ad96cf48c5533c394b4543e08922ae2d31470593)) TEST: Integration testing for collision & pbd collisions
- ([39a01c2e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/39a01c2e11d832e4726aa42c9843f1fe3d351c7d)) TEST: Add all the other collision test cases for Pbd
- ([99294cfa](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/99294cfa18c68897c2cf5b5255362eea486d14ae)) TEST: Add crevice test, tweak other PbdObjectCollisionTests
- ([478de476](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/478de4767d4f124372ab5455f03e0e82e742c96f)) TEST: addSceneObject tests
- ([4fb96509](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4fb965090a582e7daadf1e865dc1a281e55c2773)) TEST: Add hexahedral mesh .msh IO test, tests validate contents
- ([481120f5](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/481120f57fd0b041fbca420917a066d686f95ef6)) TEST: Add PointSetToCylinderCD visual test
- ([8dd88704](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8dd88704a7fe508f927f51e7ae647cdd71fcbf38)) TEST: RigidObjectCollisionTest visual testing

**Bugs**

- ([f2e7eb6a](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f2e7eb6a99742a8b29cf8a326a374342aaf1adea)) BUG: Fix capsule sdf & bounding box func
- ([97b1d3ac](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/97b1d3ac0b87f1069fb84d4383069e30d125d392)) BUG: Basis in capsule getFunctionValue should use tranpose/inverse of rotation
- ([4ff061fd](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4ff061fdffa785a332fef70c91f6fc34fd10e517)) BUG: Fix task graph cyclic issue, bad ordering in task graph PbdObjectCollision
- ([ddaaad0e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ddaaad0ed5babc2289aff3cc1fa59bc276cff77e)) BUG: Fix PbdObjectGrasping task ordering
- ([d2ef1f8e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d2ef1f8e982dcab0bb6321487ae909e343cb6f37)) BUG: Divide by zero case & empty else removed in constraints
- ([06ee4e5e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/06ee4e5eade12f690465e518f3871a131861c1db)) BUG: Remove restriction on 0 capsule length
- ([dd981c74](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dd981c74441512ccd5aa3f73c88c4c5f4f18bf88)) BUG: Fix textures when used with non PBR shading / flat, gourand, or phong
- ([4f772d35](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4f772d353c4ed66c81cd62c1fa761d01c6e772ae)) BUG: Fix segment bc coords
- ([dd1aa416](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dd1aa4166750738b200ead88b755cb6d969a921f)) BUG: Fix edge-edge closest point computation

**Documentation**

- ([0aae6ec6](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0aae6ec64813c347d542355bb127cfb55cf42cc5)) DOC: Include dashboard link in readme, update PbdModel code example
- ([2cafd83b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2cafd83b23a8049997ebc2b46f818270de5e4b4f)) DOC: Add CD support matrix
- ([4804f952](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4804f95204daaf6687ae94f4d0874a3aad943039)) DOC: Update SimulationManager documentation
- ([a37ef5e7](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a37ef5e780614a2fbf2e87c3a964257dac5c5f75)) DOC: Update README, Contributing, & Maitnance docs
- ([6fd68759](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6fd6875901c4f3ba5fee71db45bda441fd14c462)) DOC: Improve PbdModel documentation
- ([6a2d3355](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6a2d335523a8be9c4be15f5889e28cfed195e15f)) DOC: Add documentation for CapsuleToCapsuleCD
- ([6508c824](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/6508c824511f2c16ce73ad60f4c753be46a9e276)) DOC: Add thread gif & description in PbdModel

**Ben Boeckel**

**Enhancements**

- ([1e262b0b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1e262b0b60c177e3fa5a86bf405c9dc9d0204890)) ENH: Add gitlab runner support and basic configuration for Windows

**Harald Scheirich**

**Enhancements**

- ([d9085dd2](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d9085dd2f5e42c3a426d68cb146baa584a77a1ee)) ENH: Update Eigen to 3.4
- ([d2419bff](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d2419bffb3ca6690def056d03f2375b4fbd14fbe)) ENH: Update VRPN to latest version
- ([2c89fa1e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2c89fa1ea51802a80b494c7e464323066fd36029)) ENH: Add adding of multiple nodes and edges to taskgraph
- ([a543c7d6](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a543c7d61e284429f80161d3a0dfbfdc4175b07f)) ENH: Update C++ to 14
- ([3c8e7957](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/3c8e7957277161efcb03475915614799236bc6e0)) REFAC: Consistently use delegate hint for render delegate creation
- ([91bf9003](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/91bf900340005f51fd4f6064280d80f460d8bfcc)) ENH: Add Analog getter in DeviceClient
- ([bc047b8d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bc047b8d15d39bd1fde75a3e47fef5b3ebfb292c)) ENH: Split rendertests into separate CMAKE variable
- ([f2491052](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f2491052206bdc331982dd186eba296a59d84edf)) ENH: Introduce TYPE_NAME macro
- ([b961cbcd](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b961cbcd481345a5981768f078555f39f024a9dc)) ENH: Common Factory infrastructure
- ([805894b1](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/805894b1158012c4a37fd02ce461ff7d5d055632)) ENH: Add simple example to use for onboarding work
- ([62a03fde](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/62a03fdeb4f1a5d688b9fcf8d90ff21cae4f9a12)) ENH: Add const begin/end to data array
- ([cb0ecef5](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/cb0ecef546323c2bfdf4ab0054b139abc83af659)) ENH: Add second logger to enable unity to catch issue thrown by failed CHECK or FATAL messages
- ([9ac5d1a5](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/9ac5d1a5185a08c2b571a4039638db4b51b1c5a3)) ENH: Enable C# and Unity to catch exceptions posted by C++ code
- ([1f721fa8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/1f721fa8bf3f5f291af2743fd5a3a5f16c9a7051)) ENH: Unity loggers log to file as well
- ([8b1cd2a3](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/8b1cd2a306a904b3a8c1c1bb22dfe28e7b4180f9)) ENH: Avoid extra DataArray allocations on assignment
- ([101bc8ee](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/101bc8eed6efb86a1b19fc2d23acb3bf0affd765)) ENH: Additional tests for testPlaneToSphere
- ([65705e50](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/65705e503c4b998f79d6254e832b9489c0432d85)) ENH: Create Confiuration file for C#
- ([c1b00ec8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c1b00ec8939e2666a55b75493525863517fe023c)) ENH: Add Dependent options with regards to Unity build

**Testing**

- ([dc9f3bf8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/dc9f3bf84b9b0e7a823633a2a86c6d75961a6312)) TEST: Add taskgraph unit tests
- ([115f9a28](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/115f9a287ac4e9fdcb96070e7ed7bcb79a710731)) TEST: Add tests for OneToOneMap

**Bugs**

- ([b2da6894](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b2da689422ead8cbadaa2e55536030368d75c862)) BUG: Fix treatment of unsafe functions when PINNED_ARRAY is on
- ([91aeb8d9](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/91aeb8d96e56662b04fe9b7482a0821ef0c6c0eb)) BUG: Correct handling of size and mapped ptrs in new copy
- ([fdb51195](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fdb5119511cb0cb0421b6bac72f07924924ea6f3)) BUG: fix spurious failure when closing a window in visual tests on linux

**Documentation**

- ([d5f54e4b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d5f54e4b4216bcd3303ef96f4c377b5198e58805)) DOC: Add language to coding guidelines addressing published work
- ([bfee2fba](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/bfee2fba8e24af076f2679bef5438655c27c838e)) DOC: Update Coding guide to bring in better in line with current iMSTK practices
- ([47c368d1](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/47c368d1f78d29b8c136a4f9da1cc0b5b04809be)) DOC: improve documentation for factories
- ([f5a1b2f8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f5a1b2f83ae8ac78ef867a5df37350d2c73a782c)) DOC: Add documentation for coverage calc
- ([f397bccd](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f397bccd1b6d8771a5ee885e7b73e923c8711c79)) DOC: Update README

**Hong Li**

**Enhancements**

- ([ff9f2cb8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ff9f2cb8280691bee53fb76f77324b09ef8a9c63)) ENH: Add support for texture coordinates runtime modification for SurfaceMesh
- ([98f4ddc2](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/98f4ddc29b0be7cb065daad2c667428a53b2bf47)) ENH: Add PointSet to Cylinder Collision Detection

**Bugs**

- ([2ace82a9](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/2ace82a9db04059d97baadec6cf17efe86402846)) BUG: Fix Plane to Sphere collision test bug

**Jacob Moore**

**Enhancements**

- ([a8676f24](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a8676f249de6c46eebb9f560d0aae8a5107aff1c)) ENH: Pbd vs Rigid Body Haptic Grapsing
- ([28b32fad](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/28b32fadbd0a05498a5cb5337b378421d32d2271)) ENH: Adding suture constraints for penetration
- ([ac24fd7d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ac24fd7dc22021acdcc3291b0d315e9fd7b958ff)) ENH: Adding suture needle interaction
- ([16fb452b](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/16fb452b910694c40490a580d8876ac547ac4a75)) ENH: Adding suture needle object

**Testing**

- ([235ef0a9](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/235ef0a94d337ddf20e86f9ffd2d8b34161b2092)) TEST: Testing for Capsule class
- ([a171879d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a171879d0575e4508be0401cf7ea78c07bab0b40)) TEST: Testing for SurfaceMeshToCapsuleCD and Capsule class
- ([e693456e](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e693456e2129d056efe2e729166972ee1376fc17)) TEST: Simplified volume test for capsule
- ([57d80e09](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/57d80e09019a0e28f58b63bc13aa32bb3050b8ea)) TEST: Adding benchmarking to PBD method
- ([c998964f](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c998964f02fddea0089030551ef8197ca0130d9f)) TEST: Added more benchmark cases with contact and modifed based off feedback
      

**Jean-Christophe Fillion-Robin**

**Enhancements**

- ([e9cc8117](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/e9cc8117aec192b56e5249f7bb72c4207d3a2852)) COMP: Update build-system to be consistent and require VTK dependency
- ([051abb5d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/051abb5d1086cc48d1cc7136396ccef428bdb2c2)) COMP: Update imstk_find_header to keep ${package}_INCLUDE_DIR in the cache
- ([086e9b05](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/086e9b0536abd721c9212cb8e77308b0d8c4494b)) COMP: Simpler config against project in given dir setting default for <package>_LIB_DIR
- ([ebfde657](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ebfde657cd32ba490add295cb33e472b18f0827c)) COMP: Support for custom include subdirectory associated with imstk_find_header
- ([35940011](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/359400116536b2c62f7817e8769d3335fe4a7422)) ENH: Update imstkSolveDependencies module based on commontk/Artichoke@4176c39f9
- ([885c7cdf](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/885c7cdf986bd7b5a2335de02b36660b14729718)) COMP: Update imstkAddExternalProject to support specifying custom install dir
- ([fe4db2e1](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/fe4db2e1466dd07ef7ad57a81f9bf24b46b5b73c)) COMP: Add relevant vars to iMSTKConfig to streamline config against a build-tree
- ([01373ce8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/01373ce80f171ab6ea7a134ce7b79de84ae78d2d)) COMP: Update Libusb external project to support Linux
- ([5e3a7f46](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/5e3a7f463b24dcc9c18307ad6beb1c2c87a70966)) COMP: Support setting custom SOURCE_DIR and BINARY_DIR vars
- ([a30d9fa8](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a30d9fa8654e98244c70ae073f50abe27de69c11)) COMP: Update imstk_add_external_project to keep track of the project var
- ([a9203409](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/a920340960406c8dcc30c4c9785a121a11af2af4)) COMP: Streamline client project integration updating TBB from 2019_U2 to 2019_U9
- ([abadb943](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/abadb9438e4d7939c4deaed122cf0aefc7288159)) COMP: Update external project supporting setting a custom git repository
- ([17d0c63d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/17d0c63d6923103c2c57555cd48024075eaa0271)) COMP: Update imstkSolveDependencies module based on commontk/Artichoke@edfc828)
- ([663b4daf](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/663b4daf081ddd937c68cdb68c6b6c6b96b1876e)) COMP: Ensure inner build is always re-configured
- ([f8a1edb0](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f8a1edb0c97d8b4d9e022d6de4fd22bc8a46ff87)) COMP: Simplify and fix handling of PHYSX_CONFIGURATION option
- ([b21384c2](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/b21384c28e6d5e720fe62b76b996fdd712761a62)) COMP: Simplify and fix handling of iMSTK_USE_OpenHaptics option
- ([ff4d8c3d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/ff4d8c3d4823b19d1691e590d224e82644cce7cd)) COMP: Update inner build external project to use CMAKE_CACHE_ARGS
- ([4600b6c9](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4600b6c9ecf1b57734a14c6dc215830f312081fe)) COMP: Simplify and fix passing of options to inner build using mark_as_superbuild
- ([c55b3ed1](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c55b3ed11e1f1cb40f1dcb64e3b6e118e6698bd1)) COMP: Remove obsolete inner build external project patch step for VTK 8.2
- ([48004598](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/4800459833fbc88649eaaa8a3653d6ec0845aa9e)) COMP: Update Assimp to simplify packaging in client projects

**Shreeraj Jadhav**

**Enhancements**

- ([c30e56e5](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/c30e56e52e23aa0d53645f1313511c2bcf32f9bd)) ENH: Create a CCD Algorithm class
- ([406c734f](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/406c734fae88a4008a8e4547ae804fb5156bd0d7)) ENH: Add CCD algorithm for LineMeshtoLineMesh collision
- ([0a69049d](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/0a69049d8bca71f497ee099b3d0a1bce3d0306c2)) ENH: Add new contraint for CCD Edge-Edge collision
- ([f5de4123](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f5de4123d4d17bcac909bc0cd947d812443ebd32)) ENH: Collision handling for Edge-Edge CCD
- ([78d0dc32](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/78d0dc32742cbb5ce607915c50aadbc127ff1958)) ENH: Add example for LineMesh self CCD

**Testing**

- ([d01f18cc](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/d01f18ccea46695784871a48285b3bfd3e5050d7)) TEST: Add testing for LineMesh CCD

**Documentation**

- ([f1c1dced](https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/f1c1dced126cf2dae755d9000fd5813374857247)) DOC: Add description for LineMeshToLineMeshCCD

**Api Changes**

- `PbdModel::getParameters` → `PbdModel::getConfig`
- `PbdModelConfig::enableConstraint` now takes `PbdModelConfig::ConstraintGenType` uncoupling constraint generation schemes from constraint types.
    - ex: `enableConstraint(PbdConstraint::Type::Distance, 1.0e2);` -> `enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);`
- `SimulationManager` library split into `SimulationManager`, `ViewerCore`, & `ViewerVTK`. Linkage to `ViewerVTK` may be required in dependent projects should you require that library.
- Rendering library split into `RenderingCore` & `RenderingVTK`.
- `GeometryMap`s, now extend `GeometryAlgorithm`
    - `OneToOneMap` → `PointwiseMap`
    - `TetraToTriangleMap` → `TetraToPointSetMap`
    - `OneToOneMap::getIdx` → `OneToOneMap::getParentId`
    - `GeometryMap::apply` → `GeometryMap::update`
    - Geometry constructor inputs → `GeometryMap::setParentGeometry` & `GeometryMap::setChildGeometry`
    - GeometryMap::Type enum removed.
- Many acronym’d names now Upper case first letter only. ie: `PBD` → `Pbd`. `FEM` → `Fem`. Previously had mixed usage.
- `PbdPickingCH` removed/refactored into `PbdObjectGrasping`.
- All barycentric functions moved to `imstkMath.h`
- `DataLogger` removed
- `MeshToMeshBruteForceCD` renamed to `ClosedSurfaceMeshToMeshCD`
- All VTKOpenVR classes now VTKVR. ex: `VTKOpenVRViewer` → `VTKVRViewer`. Backend is unspecified in name. OpenXR by default.
- `IdentityMap` removed.
- Collision detection algorithm factory refactored. Get collision detection class by name with `CDObjectFactory::makeCollisionDetection("SurfaceMeshToCapsuleCD");`
- `VisualModel` constructor input removed. Use `VisualModel::setGeometry` instead.
- `CollisionPair`s & `CollisionGraph` removed. Use `SceneObject`’s added to the scene with `Scene::addInteraction`
- A few occurrences of `getTranslation` & `getRotation` changed to `getPosition` & `getOrientation` when referring to a pose, not a transformation.
- PhysX deprecated backend removed completely.
- Only utilized VTK libraries are linked to, not every built VTK library. Any user facing VTK code may need to link to required libraries.
- Update to C++14
- Update to VTK 9.1
- Update to TBB 2021.1.1
- Update Eigen to 3.4
- Update VRPN

**Contributors**: Andrew Wilson, Harald Scheirich, Shreeraj Jadhav, Jacob Moore, Jean-Christophe Fillion-Robin, Hong Li, Ben Boeckel

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