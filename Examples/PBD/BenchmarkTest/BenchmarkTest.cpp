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



//
/// \brief Creates PBD object of a volume mesh
/// \param name
/// \param physical dimension of block
/// \param dimensions of tetrahedral grid used for block
/// \param center of block
///
static std::shared_ptr<PbdObject>
makePbdObjVolume(const std::string& name,
           const Vec3d& size, 
           const Vec3i& dim, 
           const Vec3d& center,
           const int numIter)
{
    imstkNew<PbdObject> prismObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(size, dim, center);
    

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = numIter;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                // if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                if (y == dim[1]-1)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
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

    return prismObj;
}

//
/// \brief Creates PBD-FEM object of a volume mesh
/// \param name
/// \param physical dimension of block
/// \param dimensions of tetrahedral grid used for block
/// \param center of block
///
static std::shared_ptr<PbdObject>
makePbdFemObjVolume(const std::string& name,
           const Vec3d& size, 
           const Vec3i& dim, 
           const Vec3d& center,
           const int numIter)
{
    imstkNew<PbdObject> prismObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(size, dim, center);
    
    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use FEMTet constraints
    pbdParams->m_femParams->m_YoungModulus = 5.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4;
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);


    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = numIter;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                // if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                if (y == dim[1]-1)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
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

    return prismObj;
}

//
/// \brief Creates PBD object of a surface mesh
/// \param name
/// \param physical dimension of block
/// \param dimensions of tetrahedral grid used for block
/// \param center of block
///
static std::shared_ptr<PbdObject>
makePbdObjSurface(const std::string& name,
           const Vec3d& size, 
           const Vec3i& dim, 
           const Vec3d& center,
           const int numIter)
{
    imstkNew<PbdObject> prismObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(size, dim, center);
    std::shared_ptr<SurfaceMesh>     surfMesh   = prismMesh->extractSurfaceMesh();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -8.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = numIter;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {   
        auto position = surfMesh->getVertexPosition(vert_id);

        if (position(1) == 2.0){
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

    return prismObj;
}




// Time evolution step of PBD using Distance+Volume constraint
static void BM_DistanceVolume(benchmark::State& state) 
{
  	
  	// Setup
    imstkNew<Scene> scene("PbdBenchmark");

  	double dt = 0.001;


    state.counters["DOFs"] = state.range(0)*state.range(0)*state.range(0);


    std::shared_ptr<PbdObject> prismObj = makePbdObjVolume("Prism",
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0),
        state.range(1));



    scene->addSceneObject(prismObj);
    scene->initialize();

  	for (auto _ : state) {
        scene->advance(dt);
  	}
    // state.SetBytesProcessed(int64_t(state.iterations()) *
    //                       int64_t(state.range(0)));
}


// Time evolution step for PBD using distance+dihedral angle constraint
static void BM_DistanceDihedral(benchmark::State& state) {
    
    // Setup
    imstkNew<Scene> scene("PbdBenchmark");

    double dt = 0.001;

    // Estimate of number of DOFs
    state.counters["DOFs"] = state.range(0)*state.range(0)*6;

    std::shared_ptr<PbdObject> prismObj = makePbdObjSurface("Prism",
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0),
        state.range(1));

    scene->addSceneObject(prismObj);
    scene->initialize();

    for (auto _ : state) {
        scene->advance(dt);
    }
    // state.SetBytesProcessed(int64_t(state.iterations()) *
    //                       int64_t(state.range(0)));

}

// Time evolution step of PBD using FEM constraint
static void BM_PbdFem(benchmark::State& state) 
{
    
    // Setup
    imstkNew<Scene> scene("PbdBenchmark");

    double dt = 0.001;


    state.counters["DOFs"] = state.range(0)*state.range(0)*state.range(0);

    std::shared_ptr<PbdObject> prismObj = makePbdFemObjVolume("Prism",
        Vec3d(4.0, 4.0, 4.0), 
        Vec3i(state.range(0), state.range(0), state.range(0)), 
        Vec3d(0.0, 0.0, 0.0),
        state.range(1));

    scene->addSceneObject(prismObj);
    scene->initialize();

    for (auto _ : state) {
        scene->advance(dt);
    }
    // state.SetBytesProcessed(int64_t(state.iterations()) *
    //                       int64_t(state.range(0)));
}


// Time a simple translation of the mesh
static void BM_MeshTransform(benchmark::State& state) {
    // Setup the mesh
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(Vec3d(2.0, 2.0, 2.0),Vec3i(20, 20, 20), Vec3d(0.0, 1.0, 0.0));

    // Make sure compiler is not optimizing away the loop. 
    // This loop gets timed
    for (auto _ : state) {
        prismMesh->translate({0.1, 0.1, 0.1}, Geometry::TransformType::ApplyToData);  
    }
}


BENCHMARK(BM_DistanceVolume)
    ->Unit(benchmark::kMillisecond)
    ->Name("Cube Volume Benchmark Distance and Volume Constraints")
    ->ArgsProduct({{4,6,8,10,16,20,25}, {2, 5}});

BENCHMARK(BM_DistanceDihedral)
    ->Unit(benchmark::kMillisecond)
    ->Name("Cube Surface Benchmark Distance and Dihedral Angle Constraints")
    ->ArgsProduct({{4,6,8,10,16,20,25}, {2, 5}});

BENCHMARK(BM_PbdFem)
    ->Unit(benchmark::kMillisecond)
    ->Name("Cube Volume using FEM Constraints")
    ->ArgsProduct({{4,6,8,10,16,20}, {2, 5}});


BENCHMARK(BM_MeshTransform); 

// Run the benchmark
BENCHMARK_MAIN();

