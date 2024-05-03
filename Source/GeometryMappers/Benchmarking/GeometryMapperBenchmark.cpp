/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCapsule.h"
#include "imstkCollisionHandling.h"
#include "imstkGeometry.h"
#include "imstkMath.h"
#include "imstkMeshIO.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointwiseMap.h"
#include "imstkRbdConstraint.h"
#include "imstkScene.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkTetrahedralMesh.h"

#include <benchmark/benchmark.h>

using namespace imstk;

///
/// \brief Creates a tetraheral grid
/// \param size physical dimension of domain
/// \param dim dimensions of tetrahedral grid
/// \param center center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    auto prismMesh   = std::make_shared<TetrahedralMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(dim[0] * dim[1] * dim[2]);

    VecDataArray<double, 3>& vertices = *verticesPtr.get();
    const Vec3d              dx       = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                vertices[x + dim[0] * (y + dim[1] * z)] = Vec3i(x, y, z).cast<double>().cwiseProduct(dx) - size * 0.5 + center;
            }
        }
    }

    // Add connectivity data
    auto indicesPtr = std::make_shared<VecDataArray<int, 4>>();

    VecDataArray<int, 4>& indices = *indicesPtr.get();
    for (int z = 0; z < dim[2] - 1; z++)
    {
        for (int y = 0; y < dim[1] - 1; y++)
        {
            for (int x = 0; x < dim[0] - 1; x++)
            {
                int cubeIndices[8] =
                {
                    x + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * (z + 1))
                };

                // Alternate the pattern so the edges line up on the sides of each voxel
                if ((z % 2 ^ x % 2) ^ y % 2)
                {
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[7], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[2], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[7], cubeIndices[5], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[2], cubeIndices[0], cubeIndices[5]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[6], cubeIndices[7], cubeIndices[5]));
                }
                else
                {
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[3], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[6], cubeIndices[2], cubeIndices[1]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[6], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[3], cubeIndices[1], cubeIndices[4]));
                }
            }
        }
    }

    auto uvCoordsPtr = std::make_shared<VecDataArray<float, 2>>(dim[0] * dim[1] * dim[2]);

    VecDataArray<float, 2>& uvCoords = *uvCoordsPtr.get();
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                uvCoords[x + dim[0] * (y + dim[1] * z)] = Vec2f(static_cast<float>(x) / dim[0], static_cast<float>(z) / dim[2]) * 3.0;
            }
        }
    }

    // Ensure correct windings
    for (int i = 0; i < indices.size(); i++)
    {
        if (tetVolume(vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]]) < 0.0)
        {
            std::swap(indices[i][0], indices[i][2]);
        }
    }

    prismMesh->initialize(verticesPtr, indicesPtr);
    prismMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return prismMesh;
}

///
/// \brief Time evolution step of PBD using Distance+Volume constraint on tet mesh
///
static void
BM_CopyLoop(benchmark::State& state)
{
    std::shared_ptr<PointSet> parent = std::make_shared<PointSet>();
    std::shared_ptr<PointSet> child  = std::make_shared<PointSet>();

    const int numPoints = state.range(0);


    auto parentVerticesPtr = std::make_shared<VecDataArray<double, 3>>(numPoints);
    auto childVerticesPtr  = std::make_shared<VecDataArray<double, 3>>(numPoints);

    // randomize parent vertices
    for (int i = 0; i < numPoints; i++)
    {
		(*parentVerticesPtr)[i] = Vec3d::Random();
	}
    parent->setInitialVertexPositions(parentVerticesPtr);
    child->setInitialVertexPositions(childVerticesPtr);

    // Create the map
    
    std::vector<std::pair<int, int>> map;
    for (int i = 0; i < numPoints; i++)
    {
		map.push_back({ i, numPoints - i - 1 });
	}
    
   
    // Copy loop
    for (auto _ : state)
    {
        for (int i = 0; i < numPoints; i++)
        {
			(*childVerticesPtr)[map[i].second] = (*parentVerticesPtr)[map[i].first];
		}
	}
  
}

BENCHMARK(BM_CopyLoop)
->Unit(benchmark::kMicrosecond)
->Name("Copy vertices in loop")
->RangeMultiplier(2)->Range(8, 16 << 10);

static void
BM_CopyParallel(benchmark::State& state)
{
	std::shared_ptr<PointSet> parent = std::make_shared<PointSet>();
	std::shared_ptr<PointSet> child = std::make_shared<PointSet>();

	const int numPoints = state.range(0);


	auto parentVerticesPtr = std::make_shared<VecDataArray<double, 3>>(numPoints);
	auto childVerticesPtr = std::make_shared<VecDataArray<double, 3>>(numPoints);

	// randomize parent vertices
	for (int i = 0; i < numPoints; i++)
	{
		(*parentVerticesPtr)[i] = Vec3d::Random();
	}
	parent->setInitialVertexPositions(parentVerticesPtr);
	child->setInitialVertexPositions(childVerticesPtr);

	// Create the map

	std::vector<std::pair<int, int>> map;
	for (int i = 0; i < numPoints; i++)
	{
		map.push_back({ i, numPoints - i - 1 });
	}


    VecDataArray<double, 3>& parentVertices = *parentVerticesPtr;
    VecDataArray<double, 3>& childVertices = *childVerticesPtr;

	// Copy loop
	for (auto _ : state)
	{
        const int size = map.size();
		ParallelUtils::parallelFor(map.size(),
			[&](const size_t idx)
			{
				const auto& mapValue = map[idx];
				childVertices[mapValue.first] = parentVertices[mapValue.second];
			}, size > 8191);
	}

}

BENCHMARK(BM_CopyParallel)
->Unit(benchmark::kMicrosecond)
->Name("Copy vertices in parallel")
->RangeMultiplier(2)->Range(8, 16 << 10);


// Run the benchmark
BENCHMARK_MAIN();
