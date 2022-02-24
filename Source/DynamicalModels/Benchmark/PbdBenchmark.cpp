/*=========================================================================

    Library: iMSTK

    Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
    & Imaging in Medicine, Rensselaer Polytechnic Institute.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

=========================================================================*/


#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRbdConstraint.h"
#include "imstkScene.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMath.h"
#include "imstkGeometry.h"
#include "imstkCapsule.h"
#include "imstkSphere.h"
#include "imstkCollisionHandling.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkSurfaceMeshToCapsuleCD.h"


#include <benchmark/benchmark.h>

using namespace imstk;




///
/// \brief Creates a tetraheral grid
/// \param physical dimension of domain
/// \param dimensions of tetrahedral grid
/// \param center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<TetrahedralMesh> prismMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const Vec3d                       dx       = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
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
    imstkNew<VecDataArray<int, 4>> indicesPtr;
    VecDataArray<int, 4>&          indices = *indicesPtr.get();
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

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
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
static void BM_DistanceVolume(benchmark::State& state) 
{
  	
  	// Setup
    imstkNew<Scene> scene("PbdBenchmark");

  	double dt = 0.05;

    // Create PBD object
    imstkNew<PbdObject> prismObj("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0));

    state.counters["DOFs"] = state.range(0)*state.range(0)*state.range(0);
    state.counters["Tets"] = prismMesh->getNumTetrahedra();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use volume+distance constraints
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0)-1)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(prismMesh);
    pbdModel->configure(pbdParams);


    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdModel);


    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // This loop gets timed
  	for (auto _ : state) {
        scene->advance(dt);
  	}
}

///
/// \brief Time evolution step of PBD using distance+dihedral constraint on surface mesh
///
static void BM_DistanceDihedral(benchmark::State& state) 
{
    
    // Setup
    imstkNew<Scene> scene("PbdBenchmark");

    double dt = 0.001;

    imstkNew<PbdObject> prismObj("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0));

    std::shared_ptr<SurfaceMesh>     surfMesh   = prismMesh->extractSurfaceMesh();

    // Estimate of number of DOFs
    state.counters["DOFs"] = surfMesh->getNumVertices();
    state.counters["Tris"] = surfMesh->getNumTriangles();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -8.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {   
        auto position = surfMesh->getVertexPosition(vert_id);

        // Switch to Eigen.isApprox()
        if (position.y() == 2.0){
            pbdParams->m_fixedNodeIds.push_back(vert_id);
        }
    }
    
    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(surfMesh);
    pbdModel->configure(pbdParams);

    // Setup the Object
    prismObj->setPhysicsGeometry(surfMesh);
    prismObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(prismObj);
    scene->initialize();


    // This loop gets timed
    for (auto _ : state) {
        scene->advance(dt);
    }
}

///
/// \brief Time evolution step of PBD using FEM constraints on volume mesh
///
static void BM_PbdFem(benchmark::State& state) 
{
    
    // Setup
    imstkNew<Scene> scene("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    imstkNew<PbdObject> prismObj("Prism");

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0));

    state.counters["DOFs"] = state.range(0)*state.range(0)*state.range(0);
    state.counters["Tets"] = prismMesh->getNumTetrahedra();
    
    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use FEMTet constraints
    pbdParams->m_femParams->m_YoungModulus = 5.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4;
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);

    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                if (y == state.range(0)-1)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(prismMesh);
    pbdModel->configure(pbdParams);


    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdModel);


    // Create the scene
    scene->addSceneObject(prismObj);
    scene->initialize();

    // This loop gets timed
    for (auto _ : state) {
        scene->advance(dt);
    }
}

///
/// \brief Time evolution step of PBD using distance+volume constraint on volume mesh
/// \brief includes contact with a capsule
/// 
static void BM_PbdContactDistanceVol(benchmark::State& state)
{

    // Setup
    imstkNew<Scene> scene("PbdBenchmark");

    double dt = 0.05;

    // Create PBD object
    imstkNew<PbdObject> prismObj("Prism");

    // Setup the mesh Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0));

    // Create surface mesh for contact
    std::shared_ptr<SurfaceMesh> surfMesh = prismMesh->extractSurfaceMesh();

    // Use surface mesh for collision
    prismObj->setCollidingGeometry(surfMesh);

    // Force deformation to match between the surface and volume mesh
    prismObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap> (prismMesh, surfMesh));


    state.counters["DOFs"] = state.range(0)*state.range(0)*state.range(0);
    state.counters["Tets"] = prismMesh->getNumTetrahedra();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = state.range(1);
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < state.range(0); z++)
    {
        for (int y = 0; y < state.range(0); y++)
        {
            for (int x = 0; x < state.range(0); x++)
            {
                // if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                if (y == state.range(0)-1)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + state.range(0) * (y + state.range(0) * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(prismMesh);
    pbdModel->configure(pbdParams);


    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdModel);

    // Add Capsule for collision
    imstkNew<Capsule> capsule;
    capsule->setRadius(0.5);
    capsule->setLength(2);
    capsule->setPosition(Vec3d(0.0, -2.6, 0.0));
    capsule->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));


    // Set up collision
    std::shared_ptr<CollidingObject> collisionObj = std::make_shared<CollidingObject>("CollidingObject");
    collisionObj->setCollidingGeometry(capsule);
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

    // This loop gets timed
    for (auto _ : state) {
        scene->advance(dt);
    }

}


BENCHMARK(BM_DistanceVolume)
    ->Unit(benchmark::kMillisecond)
    ->Name("Distance and Volume Constraints: Tet Mesh")
    ->ArgsProduct({{4,6,8,10,16,20,25}, {2, 5}});

BENCHMARK(BM_DistanceDihedral)
    ->Unit(benchmark::kMillisecond)
    ->Name("Distance and Dihedral Constraints: Surface Mesh")
    ->ArgsProduct({{4,6,8,10,16,20,25}, {2, 5}});

BENCHMARK(BM_PbdFem)
    ->Unit(benchmark::kMillisecond)
    ->Name("FEM Constraints: Tet Mesh")
    ->ArgsProduct({{4,6,8,10,16,20}, {2, 5}});


BENCHMARK(BM_PbdContactDistanceVol)
    ->Unit(benchmark::kMillisecond)
    ->Name("Distance and Volume Constraints with Contact: Tet Mesh")
    ->ArgsProduct({{4,6,8,10,16,20,25}, {2, 5}});


// Run the benchmark
BENCHMARK_MAIN();

