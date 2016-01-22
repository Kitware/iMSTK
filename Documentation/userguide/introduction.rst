.. role:: cxx(code)
   :language: c++

.. role:: arg(code)
   :language: sh

.. _imstk-introduction:

********************
Introducing iMSTK
********************

iMSTK is a set of libraries, each of which provides capabilities aimed at
interactive simulations of medical procedures.
The capabilities are split into different libraries to allow for smaller
executables and fewer external dependencies for applications that use only
a fraction of them.

.. findfigure:: imstk-dataflow.*

   This diagram illustrates how data flows over time in
   a iMSTK application.

The base library provides an abstract framework for simulations, and
its core object, :sm:`smSDK`, holds references to all of the potential
resources needed for a full-fledged application:

+ A :sm:`viewer <smViewer>` that represents the top-level window used for
  rendering and user interaction;
+ A :sm:`simulator <smSimulator>` that manages how objects in a scene interact
  with each other (via collision detection and response);
+ An array of :sm:`scene <smScene>` instances that in turn own cameras, lights,
  and objects to be rendered into a scene.

============
Scene graphs
============

Although the data-flow diagram above shows only one scene graph,
you can think of iMSTK as having two separate scene graphs:

+ **(simulation scene)** one scene graph for advancing time in the simulation,
  whose objects are surface and volume meshes, collision detectors and responders,
  and time integrators; and
+ **(rendering scene)** one scene graph for rendering an illustration of the simulation,
  whose objects are surface triangulations, textures, lights, and cameras.

============================
Interactions between objects
============================

The *nodes* of a simulation scene graph represent surface meshes — and in the case of
volumetric meshes, the surface mesh that bounds the corresponding volume.
The *edges* of the simulation scene graph represent interactions between the nodes
such as collision detection and response.
When meshes are configured to interact with each other, their degrees of freedom in
the simulation must advance in lock step;
they must share a time integrator so that when a collision occurs the response
forces can be applied simultaneously with the collisions.
Otherwise, the lag between collision and response adds to instability of the integration,
forcing divergence or small timesteps.

.. _fig-scene-interactions:

.. findfigure:: imstk-mesh-interactions.*

   Two simulations with the same number of objects, but different
   interactions. On the left, all objects interact directly except
   B and D; a single time integrator must be used.
   On the right, only objects A and D interact; a single time
   integrator is used for them. A second integrator can advance
   object C, and object B does not have a time integrator since its
   motion is prescribed externally (e.g., from a haptics device).

The number of time integrators in a simulation is less than or
equal to the number of connected components in the graph of
scene object interactions.
(It can be less than this number when some objects have
prescribed spacetime paths, like :sm:`smStaticSceneObject`).
The net effect of this is illustrated with examples in
:num:`Figure #fig-scene-interactions`.

.. _fig-scene-solvers:

.. findfigure:: imstk-interaction-types.*

   Continuing the example from the *left* of
   :num:`Figure #fig-scene-interactions`,
   this figure illustrates how collision pairs can be configured
   to use different detection and response algorithms.
   The responses are then used to assemble equations of motion for
   the objects involved.

As the example makes clear, the number of collision pairs between
objects can be higher than the number of objects since in general
a graph with N nodes can have up to N*(N-1)/2 undirected edges.
In general, performance will suffer as collision pairs are added
so keeping the graph small is advised.

Once the collisions have been detected, responses can be computed
using different methods (although currently iMSTK only provides
a penalty-based response) and the reaction forces applied to the
right-hand side of the equations of motion for each of the
degrees of freedom involved.
Note that while the example in :num:`Figure #fig-scene-solvers`
shows response forces feeding into
multiple solvers, this is unadvisable since it can cause
integrator stability issues as noted above.
shows how one example

.. todo:: Finish discussing the simulation scene graph

   The scene graph needs a concrete example, whether it takes
   the form of C++ code or just a discussion of a configuration
   file is not important.

======================
Illustrating the scene
======================

Now that we've discussed the mechanics of the simulation scene graph,
let's consider the rendering scene graph.
Unlike the simulation, this graph is *bipartite*, meaning that there are
different types of nodes and edges.
The *nodes* can represent an object to render, a transform matrix, a
texture or color to apply, or any other state information used to
render the scene.
The *edges* represent inheritance; that is, each node inherits the
visual properties of its parents unless they are explicitly overridden
by the node itself.

.. todo:: Finish discussing the rendering scene graph

   This also needs a concrete example that reduces the principles
   to implementation issues.
