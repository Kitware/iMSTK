Collision Detection
===================

A collision detection method in iMSTK is defined through the ``CollisionDetectionAlgorithm`` base class. This class assumes two input iMSTK geometries and produces a CollisionData output. Whilst iMSTK geometries can be given in either order to the ``CollisionDetectionAlgorithm`` the input order will effect the output order. For example:

.. code:: c++

   imstkNew<SurfaceMeshToSphereCD> myCollisionDetection;
   myCollisionDetection->setInput(myGeomA, 0);
   myCollisionDetection->setInput(myGeomB, 1);
   myCollisionDetection->update();
   
   std::shared_ptr<CollisionData> cdData = myCollisionDetection->getCollisionData();
   cdData->elementsA; // Refers to input 0 geometry
   cdData->elementsB; // Refers to input 1 geometry


Collision Detection Method Types
--------------------------------

There are two approaches to collision detection.

**Static**: Given a snapshot of two already overlapping geometries, find the best solution to separate them. This normally involves resolving the geometry along directions that produce minimal movement. Often objects in physics simulations move small amounts so we can reliably resolve along this direction to get believable behavior.

- If our objects move to fast, we might see some strange results. Such as objects tunneling through each other without ever observing a collision. Or geometry resolving to the wrong feature of another geometry. This also effects the maximum allowed velocity or force of a simulation. Which can make your simulation harder to tune (smaller valid parameter space).

**Dynamic**: Given two overlapping geometries and their dynamics (or two cached snapshots of them over time). Find the intersections a-priori and deal with them. Almost 100% of the time dynamic CD methods are paired with static CD methods as a fallback. This allows solvers to not guarantee non-intersecting states at the end of a timestep which may be difficult (for example, in an over constrained system).

- **CCD**: Continuous collision detection is a form of dynamic CD. It uses the dynamics of the object to compute exact time of impact. The usual solution is to linear interpolate elements of a geometry over time and find the time at which the elements are coplanar. For example, a vertex and a triangle moving. When the vertex and triangle are coplanar they intersect so long as the vertex still lies in the triangle. CCD can often be hard to tune and deal with floating point error. This can be alleviated with static CD as a fallback.

Collision Manifolds
--------------------------------

Collision manifolds define the areas/points/patches of contact should two bodies be separated and *just* touch. From there you can compute contacts or whatever you need for your solver. These manifolds are made up of N or more types of contacts. We have various types of contacts for shapes.

.. figure:: media/Collision_Detection/contactManifolds.png
    :width: 600
    :alt: Alternative text
    :align: center

    Various manifolds

* Face-Vertex:

  * For triangle meshes, popularly just called vertex-triangle or VT/TV contact.
  
* Face-Face:
  
  * Ignored, covered with Face-Vertex
  * For a face to be on the other side of another face and inside the shape all the vertices of said face must be inside the other shape
  
* Face-Edge:
  
  * Ignored, covered with Face-Vertex.
  
* Edge-Edge:
* Edge-Vertex:
  
  * Ignored, covered with Face-Vertex.
  * Required when using curved surface vs meshes.
  
* Vertex-Vertex:
  
  * Ignored, covered with Face-Vertex.
  * Required when using curved surface vs meshes.

How a collision manifold is specified varies a lot among collision systems. A problem with finding the collision manifold in static CD is that you actually have an overlapping volume when looking at a snapshot of intersections:

.. figure:: media/Collision_Detection/edgeContactOverlap.png
    :width: 200
    :alt: Alternative text
    :align: center

    Manifold when overlapping

.. figure:: media/Collision_Detection/edgeContact.png
    :width: 200
    :alt: Alternative text
    :align: center

    Manifold when separated

Then it becomes of a problem of specifying this to your solver. Some collision systems report intersecting elements (ie: This triangle touched this edge, or this edge touched this point). Others report per contact points where N point-based contacts need to be used to support a face.

.. figure:: media/Collision_Detection/edgeContactResolve.png
    :width: 300
    :alt: Alternative text
    :align: center

    Two point,direction,depth contacts required to support the box

Instead of subbing points for faces though, we can directly formulate a constraint between two elements in contact. For example, Vertex-Triangle as mentioned earlier as below:

.. image:: media/Collision_Detection/vertexTriangle.png
    :width: 300
    :alt: Alternative text
    :align: center

Collision Data
--------------------------------

With this approach it is required to store contact pairs of elements. Vertex-triangle, edge-edge. Whereas in the previous approach we can only store point-based contacts. iMSTK supports both methods providing the following element types.

* **PointDirectionElement**: Gives point, normal, and depth contact
* **CellIndexElement**: Gives the id of a cell or the id of a cells vertices. Check count to tell which.

  * If idCount==1. The id refers to a cell given by type.

    * IMSTK_VERTEX
    * IMSTK_EDGE
    * IMSTK_TRIANGLE
    * IMSTK_TETRA

  * If idCount > 1. The id refers to the vertex indices of the cell.

     * ex: idCount==3, means 3 vertex ids of the triangle.
     * The ability to give cells via vertex ids is useful to avoid assigning ids to cells of cells. ie: edges of triangles, triangle faces of a tetrahedron, edges of a tetrahedron.

* **CellVertexElement**: Same as a CellIndexElement but gives the vertices by value instead.

  * Useful when the other geometry doesn't contain vertices with ids (implicit geometry).

iMSTK collision methods prefer to produce contact element pairs over point-based contacts. This is because point-based contacts can be computed from element pairs when needed. But element pairs cannot so easily be computed from point-based contacts.

Collision Resolution
--------------------------------

To resolve collision I would classify approaches into two categories.

* Matrix-Free: These approaches normally resolve collisions at the moment of finding them. Or in a later iteration over all contacts found during collision.

  * Ex1: A point lies under a plane 50 units. At the moment of noticing, we move it up 50 units.
  * Ex2: A point lies under a plane 50 units. We add a contact that informs us to move it up 50 units. We later resolve all contacts.
  * If we resolve all contacts later we may find that we resolve one such that we create another. For example, stacked cubes A, B, & C. Resolving A-B might move B into C. This would normally require another collision detection pass (likely next step of the simulation). But if you noticed, Ex1 may not require another CD iteration as it does CD while resolving. Given the correct order of CD testing, they would actually resolve.

* Matrix: These approaches assemble matrices to resolve them all in a semi-implicit or implicit manner.

   * Semi-Implicit:

     * Identical to the matrix-free explicit solutions. ex1 was a one iteration of gauss seidel, ex2 was one iteration of jacobi.
     * Non-penetration equations are solved in iterative manners, solutions being plugged into latter iterations.
     * Order of assembly matters.

   * Implicit:

     * All non-penetration equations solved simulatenously.
     * Order of assembly will produce same solution.

Collision Constraints
--------------------------------

The matrix ones are often "constraint based". The constraints giving a single scalar and gradient for which to solve. Often represented as a single row in our system. For a non-penetration constraint the scalar should be 0 when separated. The gradient then gives you the direction to change the inputs such that you would reach a scalar of 0. This gives us a bit of a better generalization to apply it to a lot of things, perhaps not even non-penetration constraints (springs, joints, etc).

* PBDCollisionConstraints: Given positions of geometry, computes a gradient and scalar to reach zero/non-penetration.
* RBDConstraint: Given position and orientation of body, computes a jacobian (linear and angular gradient) and scalar to reach zero/non-penetration.

A pbd constraint to keep a point to a plane:

* Scalar = distance between the plane and point
* Gradient = the plane normal (direction to resolve, direction to get to a scalar of 0)

.. image:: media/Collision_Detection/constraintEx1.png
    :width: 300
    :alt: Alternative text
    :align: center

A rbd constraint to keep a box above a plane by adding a constraint per vertex corner.

* Scalar = distance between plane and vertex/corner of box.
* jacobian
  * Linear Gradient = plane normal, direction to resolve linearly
  * Angular Gradient = plane normal crossed with the distance between point and center of box mass, direction to resolve angularly