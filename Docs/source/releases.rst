========
Releases
========

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

Venkata Sreekanth Arikatla
Nghia Truong
Nicholas Boris Milef 
Aaron Bray
Ruiliang Gao
Johan Andruejol