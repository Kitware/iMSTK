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

LocalMarchingCubes
-----------------------

Local marching cubes does marching cubes in chunks on a single ImageData. This requires no reordering of the ImageData in memory, and no special accessors. It produces N SurfaceMesh's, one for each chunk. The purpose of LocalMarchingCubes is to avoid remeshing the entire domain reducing computational cost.

To properly divide an image the dimensions - 1 must be divisible by the number of chunks. So if you're image is 100, 100, 100. Then you need to use divisors of 99, 99, 99. If what you provided is not a divisor it will choose the next divisor and warn you.

::

	imstkNew<LocalMarchingCubes> isoExtract;
	isoExtract->setInputImage(inputImage);
	isoExtract->setIsoValue(0.0);
	isoExtract->setNumberOfChunks(Vec3i(32, 9, 9));
	isoExtract->update();

	// Output given, some chunks may have no vertices
	for (int i = 0; i < 39 * 9 * 9; i++)
	{
		isoExtract->getOutput(i);
	}

Below shows the level set cutting method with chunked marching cubes.

.. image:: media/localmc.gif
	:width: 700
	:alt: Chunked marching cubes