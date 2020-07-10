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

#include "imstkCamera.h"
#include "imstkCollisionGraph.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraTriangleMap.h"
#include "imstkVisualModel.h"

using namespace imstk;

// mesh file names
const std::string surfMeshFileName = iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj";
const std::string tetMeshFileName  = iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg";
///
/// \brief Create a surface mesh
/// \param nRows number of vertices in x-direction
/// \param nCols number of vertices in y-direction
///
std::shared_ptr<SurfaceMesh> createUniformSurfaceMesh(const double width, const double height, const size_t nRows, const size_t nCols);

// parameters to play with
const double youngModulus     = 1000.0;
const double poissonRatio     = 0.3;
const double timeStep         = 0.01;
const double contactStiffness = 0.1;
const int    maxIter = 5;

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamics
///
int
main()
{
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("PbdCollisionOneDragon");
    scene->getCamera()->setPosition(0, 3.0, 20.0);
    scene->getCamera()->setFocalPoint(0.0, -10.0, 0.0);

    // set up the meshes
    auto highResSurfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(surfMeshFileName));
    auto coarseTetMesh   = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(tetMeshFileName));
    auto coarseSurfMesh  = std::make_shared<SurfaceMesh>();
    coarseTetMesh->extractSurfaceMesh(coarseSurfMesh, true);

    // set up maps
    auto mapPhysicsToVisual    = std::make_shared<TetraTriangleMap>(coarseTetMesh, highResSurfMesh);
    auto mapCollisionToPhysics = std::make_shared<OneToOneMap>(coarseTetMesh, coarseSurfMesh);

    // set up visual model based on high res mesh
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setLineWidth(0.5);
    material->setEdgeColor(Color::Blue);
    material->setShadingModel(RenderMaterial::ShadingModel::Phong);
    auto surfMeshModel = std::make_shared<VisualModel>(highResSurfMesh);
    surfMeshModel->setRenderMaterial(material);

    // configure the deformable object
    auto deformableObj = std::make_shared<PbdObject>("DeformableObj");
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setCollidingGeometry(coarseSurfMesh);
    deformableObj->setPhysicsGeometry(coarseTetMesh);
    deformableObj->setPhysicsToCollidingMap(mapCollisionToPhysics);
    deformableObj->setPhysicsToVisualMap(mapPhysicsToVisual);

    // Create model and object
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(coarseTetMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // FEM constraint
    pbdParams->m_femParams->m_YoungModulus = youngModulus;
    pbdParams->m_femParams->m_PoissonRatio = poissonRatio;
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet,
                                   PbdFEMConstraint::MaterialType::Corotation);

    // Other parameters
    // \todo use lumped mass
    pbdParams->m_uniformMassValue = 1.0;

    pbdParams->m_gravity    = Vec3d(0, -10.0, 0);
    pbdParams->m_defaultDt  = timeStep;
    pbdParams->m_iterations = maxIter;
    pbdParams->collisionParams->m_proximity = 0.3;
    pbdParams->collisionParams->m_stiffness = 0.1;

    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);

    // Build floor geometry
    auto floorMesh = createUniformSurfaceMesh(100.0, 100.0, 2, 2);

    auto materialFloor = std::make_shared<RenderMaterial>();
    materialFloor->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto floorMeshModel = std::make_shared<VisualModel>(floorMesh);
    floorMeshModel->setRenderMaterial(materialFloor);

    auto floor = std::make_shared<PbdObject>("Floor");
    floor->setCollidingGeometry(floorMesh);
    floor->setVisualGeometry(floorMesh);
    floor->setPhysicsGeometry(floorMesh);

    auto pbdModel2 = std::make_shared<PbdModel>();
    pbdModel2->setModelGeometry(floorMesh);

    // configure model
    auto pbdParams2 = std::make_shared<PBDModelConfig>();
    pbdParams2->m_uniformMassValue = 0.0;
    pbdParams2->m_iterations       = 0;
    pbdParams2->collisionParams->m_proximity = -0.1;

    // Set the parameters
    pbdModel2->configure(pbdParams2);
    floor->setDynamicalModel(pbdModel2);

    scene->addSceneObject(floor);

    // Collision
    scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(deformableObj, floor,
        InteractionType::PbdObjToPbdObjCollision, CollisionDetection::Type::MeshToMeshBruteForce));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Paused);

    return 0;
}

std::shared_ptr<SurfaceMesh>
createUniformSurfaceMesh(const double width, const double height, const size_t nRows, const size_t nCols)
{
    const double dy = width / static_cast<double>(nCols - 1);
    const double dx = height / static_cast<double>(nRows - 1);

    StdVectorOfVec3d vertList;
    vertList.resize(nRows * nCols);

    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; j++)
        {
            const double y = static_cast<double>(dy * j);
            const double x = static_cast<double>(dx * i);
            vertList[i * nCols + j] = Vec3d(x - height * 0.5, -10.0, y - width * 0.5);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i* nCols + j, i* nCols + j + 1, (i + 1) * nCols + j } };
            tri[1] = { { (i + 1) * nCols + j + 1, (i + 1) * nCols + j, i* nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    auto surfMesh = std::make_shared<SurfaceMesh>();
    surfMesh->initialize(vertList, triangles);
    return surfMesh;
}
