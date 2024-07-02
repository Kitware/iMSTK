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
#include "imstkGeometryUtilities.h"

#include <benchmark/benchmark.h>

using namespace imstk;

std::shared_ptr<SurfaceMesh>
makeSurfaceMesh(int dim)
{
    return GeometryUtils::toTriangleGrid(Vec3d::Zero(), Vec2d{ 1, 1 }, Vec2i{ dim, dim });
}

///
/// \brief Time evolution step of PBD using Distance+Volume constraint on tet mesh
///
static void
BM_SurfaceMeshToCapsuleCD(benchmark::State& state)
{
    auto mesh = makeSurfaceMesh(state.range(0));
    //std::cout << "Range: " << state.range(0) << "Tris: " << mesh->getNumTriangles() << std::endl;
    //std::cout << mesh->getNumVertices() << "/" << mesh->getNumTriangles() << std::endl;

    auto capsule = std::make_shared<Capsule>(Vec3d{ 0, 0, 0 }, 0.005, 0.01);

    SurfaceMeshToCapsuleCD cd;
    cd.setInputGeometryA(mesh);
    cd.setInputGeometryB(capsule);
    cd.setGenerateCD(true, true);

    // Copy loop
    for (auto _ : state)
    {
        cd.update();
    }
}

BENCHMARK(BM_SurfaceMeshToCapsuleCD)
->Unit(benchmark::kMicrosecond)->Arg(4)->Arg(6)->Arg(8)->Arg(10)->Arg(12)->Arg(16)->Arg(24)->Arg(32)->Arg(48)->Arg(62)->Arg(78)->Arg(100);

// Run the benchmark
BENCHMARK_MAIN();
