/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkCollisionHandling.h"
#include "imstkGeometry.h"
#include "imstkMath.h"
#include "imstkMeshIO.h"
#include "imstkPbdSystem.h"
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
BM_DistanceVolume(benchmark::State& state)
{
    // Setup simulation
    std::shared_ptr<Scene> scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    std::shared_ptr<PbdObject> prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Setup the Parameters
    std::shared_ptr<PbdModelConfig> pbdParams = std::make_shared<PbdModelConfig>();
    // Use volume+distance constraints
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = state.range(0) * state.range(0) * state.range(0);
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_DistanceVolume)
->Unit(benchmark::kMillisecond)
->Name("Distance and Volume Constraints: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

///
/// \brief Time evolution step of PBD using distance+dihedral constraint on surface mesh
///
static void
BM_DistanceDihedral(benchmark::State& state)
{
    // Setup
    auto   scene = std::make_shared<Scene>("PbdBenchmark");
    double dt    = 0.05;

    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    std::shared_ptr<SurfaceMesh> surfMesh = prismMesh->extractSurfaceMesh();

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use distance+dihedral angle
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -8.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(surfMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {
        auto position = surfMesh->getVertexPosition(vert_id);
        if (position.y() == 2.0)
        {
            prismObj->getPbdBody()->fixedNodeIds.push_back(vert_id);
        }
    }

    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = surfMesh->getNumVertices();
    state.counters["Tris"]       = surfMesh->getNumTriangles();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_DistanceDihedral)
->Unit(benchmark::kMillisecond)
->Name("Distance and Dihedral Constraints: Surface Mesh")
->ArgsProduct({ { 4, 8, 10, 16, 26, 38 }, { 2, 5, 8 } });
// ->ArgsProduct({{4,6,8,10,16,20}, {2, 5, 8}});

///
/// \brief Time evolution step of PBD using FEM constraints (StVK) on volume mesh
///
static void
BM_PbdFemStVK(benchmark::State& state)
{
    // Setup simulation
    auto scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEM Tet constraints
    pbdParams->m_secParams->m_YoungModulus = 5.0;
    pbdParams->m_secParams->m_PoissonRatio = 0.4;
    pbdParams->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = prismMesh->getNumVertices();
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdFemStVK)
->Unit(benchmark::kMillisecond)
->Name("FEM StVK Constraints: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

///
/// \brief Time evolution step of PBD using FEM constraints (Corotation) on volume mesh
///
static void
BM_PbdFemCorotation(benchmark::State& state)
{
    // Setup simulation
    auto scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEM Tet constraints
    pbdParams->m_secParams->m_YoungModulus = 5.0;
    pbdParams->m_secParams->m_PoissonRatio = 0.4;
    pbdParams->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::Corotation);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = prismMesh->getNumVertices();
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdFemCorotation)
->Unit(benchmark::kMillisecond)
->Name("FEM Corotation Constraints: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

///
/// \brief Time evolution step of PBD using FEM constraints (NeoHookean) on volume mesh
///
static void
BM_PbdFemNeoHookean(benchmark::State& state)
{
    // Setup simulation
    auto scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEM Tet constraints
    pbdParams->m_secParams->m_YoungModulus = 5.0;
    pbdParams->m_secParams->m_PoissonRatio = 0.4;
    pbdParams->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::NeoHookean);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = prismMesh->getNumVertices();
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdFemNeoHookean)
->Unit(benchmark::kMillisecond)
->Name("FEM NeoHookean Constraints: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

///
/// \brief Time evolution step of PBD using FEM constraints (Linear) on volume mesh
///
static void
BM_PbdFemLinear(benchmark::State& state)
{
    // Setup simulation
    auto scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEM Tet constraints
    pbdParams->m_secParams->m_YoungModulus = 5.0;
    pbdParams->m_secParams->m_PoissonRatio = 0.4;
    pbdParams->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::Linear);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = prismMesh->getNumVertices();
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdFemLinear)
->Unit(benchmark::kMillisecond)
->Name("FEM Linear Constraints: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

///
/// \brief Time evolution step of PBD using distance+volume constraint on volume mesh
/// includes contact with a capsule
///
static void
BM_PbdContactDistanceVol(benchmark::State& state)
{
    // Setup simulation
    auto scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the mesh Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Create surface mesh for contact
    std::shared_ptr<SurfaceMesh> surfMesh = prismMesh->extractSurfaceMesh();

    // Use surface mesh for collision
    auto prismCollider = std::make_shared<Collider>();
    prismCollider->setGeometry(surfMesh);
    prismObj->addComponent(prismCollider);

    // Force deformation to match between the surface and volume mesh
    prismObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(prismMesh, surfMesh));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use volume+distance
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 0.9);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.9);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0 / (double)state.range(0), 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Add Capsule for collision
    auto capsule = std::make_shared<Capsule>();
    capsule->setRadius(0.5);
    capsule->setLength(2);
    capsule->setPosition(Vec3d(0.0, -2.6, 0.0));
    capsule->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));

    // Set up collision
    auto collider = std::make_shared<Collider>();
    collider->setGeometry(capsule);
    auto collisionObj = std::make_shared<SceneObject>("CollidingObject");
    collisionObj->setVisualGeometry(capsule);
    collisionObj->addComponent(collider);
    scene->addSceneObject(collisionObj);

    std::shared_ptr<PbdObjectCollision> pbdInteraction =
        std::make_shared<PbdObjectCollision>(prismObj, collisionObj, "SurfaceMeshToCapsuleCD");
    pbdInteraction->setFriction(0.0);
    pbdInteraction->setRestitution(0.0);

    scene->addInteraction(pbdInteraction);

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Set output results
    state.counters["DOFs"]       = prismMesh->getNumVertices();
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdContactDistanceVol)
->Unit(benchmark::kMillisecond)
->Name("Distance and Volume Constraints with Contact: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

///
/// \brief Time evolution step of PBD using distance+dihedral constraint on surface mesh
/// includes contact with a capsule
static void
BM_PbdContactDistanceDihedral(benchmark::State& state)
{
    // Setup
    auto   scene = std::make_shared<Scene>("PbdBenchmark");
    double dt    = 0.05;

    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Create surface mesh for contact
    std::shared_ptr<SurfaceMesh> surfMesh = prismMesh->extractSurfaceMesh();

    // Use surface mesh for collision
    auto prismCollider = std::make_shared<Collider>();
    prismCollider->setGeometry(surfMesh);
    prismObj->addComponent(prismCollider);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Set up distance+dihearal angle constraints
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.9);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.9);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -2.0 / (double)state.range(0), 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(surfMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {
        auto position = surfMesh->getVertexPosition(vert_id);

        // Switch to Eigen.isApprox()
        if (position.y() == 2.0)
        {
            prismObj->getPbdBody()->fixedNodeIds.push_back(vert_id);
        }
    }

    // Add Capsule for collision
    auto capsule = std::make_shared<Capsule>();
    capsule->setRadius(0.5);
    capsule->setLength(2);
    capsule->setPosition(Vec3d(0.0, -2.6, 0.0));
    capsule->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));

    // Set up collision
    auto collider = std::make_shared<Collider>();
    collider->setGeometry(capsule);
    auto collisionObj = std::make_shared<SceneObject>("CollidingObject");
    collisionObj->setVisualGeometry(capsule);
    collisionObj->addComponent(collider);
    scene->addSceneObject(collisionObj);

    std::shared_ptr<PbdObjectCollision> pbdInteraction =
        std::make_shared<PbdObjectCollision>(prismObj, collisionObj, "SurfaceMeshToCapsuleCD");
    pbdInteraction->setFriction(0.0);
    pbdInteraction->setRestitution(0.0);

    scene->addInteraction(pbdInteraction);

    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = surfMesh->getNumVertices();
    state.counters["Tris"]       = surfMesh->getNumTriangles();
    state.counters["Iterations"] = state.range(1);

    // Note: Do this for other cases: call 4-5
    // scene->advance(dt);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdContactDistanceDihedral)
->Unit(benchmark::kMillisecond)
->Name("Distance and Dihedral Angle Constraints with Contact: Surface Mesh")
->ArgsProduct({ { 4, 8, 10, 16, 26, 38 }, { 2, 5, 8 } });
//->ArgsProduct({{4,6,8,10,16,20}, {2, 5, 8}});

///
/// \brief Time evolution step of PBD using FEM constraints on volume mesh
/// includes contact with capsule
static void
BM_PbdFemContact(benchmark::State& state)
{
    // Setup simulation
    auto scene = std::make_shared<Scene>("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    auto prismObj = std::make_shared<PbdObject>("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0),
        Vec3i(state.range(0), state.range(0), state.range(0)),
        Vec3d(0.0, 0.0, 0.0));

    // Create surface mesh for contact
    std::shared_ptr<SurfaceMesh> surfMesh = prismMesh->extractSurfaceMesh();

    // Use surface mesh for collision
    auto prismCollider = std::make_shared<Collider>();
    prismCollider->setGeometry(surfMesh);
    prismObj->addComponent(prismCollider);

    // Force deformation to match between the surface and volume mesh
    prismObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(prismMesh, surfMesh));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEMTet constraints
    pbdParams->m_secParams->m_YoungModulus = 5.0;
    pbdParams->m_secParams->m_PoissonRatio = 0.4;
    pbdParams->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdSystem);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0) - 1)
                {
                    prismObj->getPbdBody()->fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Add Capsule for collision
    auto capsule = std::make_shared<Capsule>();
    capsule->setRadius(0.5);
    capsule->setLength(2);
    capsule->setPosition(Vec3d(0.0, -2.6, 0.0));
    capsule->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));

    // Set up collision
    auto collider = std::make_shared<Collider>();
    collider->setGeometry(capsule);
    auto collisionObj = std::make_shared<SceneObject>("CollidingObject");
    collisionObj->addComponent(collider);
    collisionObj->setVisualGeometry(capsule);
    scene->addSceneObject(collisionObj);

    std::shared_ptr<PbdObjectCollision> pbdInteraction =
        std::make_shared<PbdObjectCollision>(prismObj, collisionObj, "SurfaceMeshToCapsuleCD");
    pbdInteraction->setFriction(0.0);
    pbdInteraction->setRestitution(0.0);

    scene->addInteraction(pbdInteraction);

    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // Setup outputs for results
    state.counters["DOFs"]       = prismMesh->getNumVertices();
    state.counters["Tets"]       = prismMesh->getNumTetrahedra();
    state.counters["Iterations"] = state.range(1);

    // This loop gets timed
    for (auto _ : state)
    {
        scene->advance(dt);
    }
}

BENCHMARK(BM_PbdFemContact)
->Unit(benchmark::kMillisecond)
->Name("FEM Constraints with contact: Tet Mesh")
->ArgsProduct({ { 4, 6, 8, 10, 16, 20 }, { 2, 5, 8 } });

// Run the benchmark
BENCHMARK_MAIN();
