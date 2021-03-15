Filtering
=========

The filtering module in iMSTK provides classes that take input geometry (or geometries) and produces output geometry (or geometries). General usage looks something like the following:

::

	// mesh1 and mesh2 are instances of SurfaceMesh
	imstkNew<AppendMesh> appendMesh;
	appendMesh->setInput(mesh1, 0);
	appendMesh->setInput(mesh2, 1);
	appendMesh->update(); // Perform the append operation

	auto results =	std::dynamic_pointer_cast<SurfaceMesh>(appendMesh->getOutput());

	// Often convenience methods are provide to cast for you
	results = appendMesh->getOutputMesh();

This particular filter takes two input SurfaceMeshes and combines them.

This pattern can be applied to many geometric operations. Many filters wrap 3rd party APIs (such as VTK) to achieve an operation. A full list can be found in files `here <https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/Source/Filtering>`__.