///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//	  http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#include "imstkCamera.h"
#include "imstkCollisionGraph.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkObjectInteractionFactory.h"
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

///
/// \brief Creates a non-manifold top part of a staircase
//.
static std::unique_ptr<SurfaceMesh>
buildStairs(int nSteps, double width, double height, double depth)
{
    // Build stair geometry
    const double halfWidth  = width * 0.5;
    const double halfHeight = height * 0.5;
    const double halfDepth  = depth * 0.5;
    const double dz = depth / static_cast<double>(nSteps);
    const double dy = height / static_cast<double>(nSteps);

    // Create vertices
    StdVectorOfVec3d vertList;
    // 4 verts per step, 2 back, then 2 bottom
    vertList.reserve(nSteps * 4 + 4);
    for (size_t i = 0; i < nSteps; i++)
    {
        const double z  = static_cast<double>(dz * i) - halfDepth;
        const double y1 = static_cast<double>(dy * i) - halfHeight;
        vertList.push_back(Vec3d(-halfWidth, y1, z));
        vertList.push_back(Vec3d(halfWidth, y1, z));

        const double y2 = static_cast<double>(dy * (i + 1)) - halfHeight;
        vertList.push_back(Vec3d(-halfWidth, y2, z));
        vertList.push_back(Vec3d(halfWidth, y2, z));
    }
    {
        const double z    = static_cast<double>(dz * nSteps) - halfDepth;
        const double yTop = static_cast<double>(dy * nSteps) - halfHeight;
        vertList.push_back(Vec3d(-halfWidth, yTop, z));
        vertList.push_back(Vec3d(halfWidth, yTop, z));

        const double yBot = -halfHeight;
        vertList.push_back(Vec3d(-halfWidth, yBot, z));
        vertList.push_back(Vec3d(halfWidth, yBot, z));
    }

    // Create cells
    std::vector<SurfaceMesh::TriangleArray> triangles;
    // Create sides and tops of steps
    for (std::size_t i = 0; i < nSteps; ++i)
    {
        // Stair front side
        triangles.push_back({ { i* 4 + 3, i* 4 + 1, i* 4 } });
        triangles.push_back({ { i* 4 + 2, i* 4 + 3, i* 4 } });
        // Stair top
        triangles.push_back({ { (i + 1) * 4, i * 4 + 3, i * 4 + 2 } });
        triangles.push_back({ { (i + 1) * 4, (i + 1) * 4 + 1, i * 4 + 3 } });
    }

    std::unique_ptr<SurfaceMesh> stairMesh = std::make_unique<SurfaceMesh>();
    stairMesh->initialize(vertList, triangles);
    return stairMesh;
}

static std::shared_ptr<PbdObject>
makeArmadilloPbdObject(const std::string& name)
{
    auto pbdObj = std::make_shared<PbdObject>(name);

    // Read in the armadillo mesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "armadillo/armadillo_volume.vtk"));
    tetMesh->scale(0.07, Geometry::TransformType::ApplyToData);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), 1.3, Geometry::TransformType::ApplyToData);
    tetMesh->translate(Vec3d(0.0f, 10.0f, 0.0f), Geometry::TransformType::ApplyToData);
    auto surfMesh = std::make_shared<SurfaceMesh>();
    tetMesh->extractSurfaceMesh(surfMesh, true);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PBDModelConfig>();
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet,
        PbdFEMConstraint::MaterialType::StVK);
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0, -10.0, 0);
    pbdParams->m_defaultDt  = 0.01;
    pbdParams->m_iterations = 5;
    pbdParams->collisionParams->m_proximity = 0.3;
    pbdParams->collisionParams->m_stiffness = 0.1;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tetMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Setup the Object
    pbdObj->addVisualModel(surfMeshModel);
    pbdObj->setCollidingGeometry(surfMesh);
    pbdObj->setPhysicsGeometry(tetMesh);
    pbdObj->setPhysicsToVisualMap(std::make_shared<TetraTriangleMap>(tetMesh, surfMesh));
    pbdObj->setDynamicalModel(pbdModel);

    return pbdObj;
}

static std::shared_ptr<PbdObject>
makeStairsPbdObject(const std::string& name, int numSteps, double width, double height, double depth)
{
    auto stairObj = std::make_shared<PbdObject>(name);

    std::shared_ptr<SurfaceMesh> stairMesh(std::move(buildStairs(numSteps, width, height, depth)));

    // Setup the parameters
    auto pbdParams = std::make_shared<PBDModelConfig>();
    pbdParams->m_uniformMassValue = 0.0;
    pbdParams->collisionParams->m_proximity = -0.1;
    pbdParams->m_iterations = 0;

    // Setup the model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(stairMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto stairMaterial = std::make_shared<RenderMaterial>();
    stairMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto stairMeshModel = std::make_shared<VisualModel>(stairMesh);
    stairMeshModel->setRenderMaterial(stairMaterial);

    stairObj->addVisualModel(stairMeshModel);
    stairObj->setDynamicalModel(pbdModel);
    stairObj->setCollidingGeometry(stairMesh);
    stairObj->setVisualGeometry(stairMesh);
    stairObj->setPhysicsGeometry(stairMesh);

    return stairObj;
}

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamic on a more complex mesh
///
int
main()
{
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("PbdStairsCollision");
    scene->getCamera()->setPosition(0.0, 0.0, -30.0);
    scene->getCamera()->setFocalPoint(0.0, 0.0, 0.0);

    // Create and add the dragon to the scene
    auto pbdDragon1 = makeArmadilloPbdObject("PbdArmadillo1");
    scene->addSceneObject(pbdDragon1);

    auto stairObj = makeStairsPbdObject("PbdStairs", 12, 20.0, 10.0, 20.0);
    scene->addSceneObject(stairObj);

    // Collision
    scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(pbdDragon1, stairObj,
        InteractionType::PbdObjToPbdObjCollision, CollisionDetection::Type::MeshToMeshBruteForce));

    // Light
    auto light = std::make_shared<DirectionalLight>("Light");
    light->setFocalPoint(Vec3d(5.0, -8.0, 5.0));
    light->setIntensity(1.0);
    scene->addLight(light);

    auto light2 = std::make_shared<DirectionalLight>("light 2");
    light2->setFocalPoint(-Vec3d(5, -8, 5));
    light2->setIntensity(1.2);
    scene->addLight(light2);

    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Paused);

    return 0;
}
