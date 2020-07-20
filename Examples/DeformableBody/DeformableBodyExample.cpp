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

#include "imstkBackwardEuler.h"
#include "imstkCamera.h"
#include "imstkFeDeformableObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"

using namespace imstk;

std::shared_ptr<DynamicObject> createAndAddFEDeformable(std::shared_ptr<Scene> scene, std::shared_ptr<TetrahedralMesh> tetMesh);

const std::string meshFileName = iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg";

///
/// \brief This example demonstrates the soft body simulation
/// using Finite elements
///
int
main()
{
    // simManager and Scene
    auto simConfig = std::make_shared<SimManagerConfig>();
    simConfig->simulationMode = SimulationMode::Rendering;
    auto simManager = std::make_shared<SimulationManager>(simConfig);
    auto scene      = simManager->createNewScene("DeformableBodyFEM");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(meshFileName);
    CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

    // Scene object 1: fe-FeDeformableObject
    auto deformableObj = createAndAddFEDeformable(scene, tetMesh);
    auto dynaModel     = std::dynamic_pointer_cast<FEMDeformableBodyModel>(deformableObj->getDynamicalModel());

    // Scene object 2: Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setPosition(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Paused);

    return 0;
}

std::shared_ptr<DynamicObject>
createAndAddFEDeformable(std::shared_ptr<Scene>    scene,
                         std::shared_ptr<TetrahedralMesh> tetMesh)
{
    auto surfMesh = std::make_shared<SurfaceMesh>();
    tetMesh->extractSurfaceMesh(surfMesh, true);

    // Configure dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    auto config    = std::make_shared<FEMModelConfig>();
    config->m_fixedNodeIds = { 50, 126, 177 };
    dynaModel->configure(config);
    //dynaModel->configure(iMSTK_DATA_ROOT "/asianDragon/asianDragon.config");

    dynaModel->setTimeStepSizeType(TimeSteppingType::Fixed);
    dynaModel->setModelGeometry(tetMesh);
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.01);// Create and add Backward Euler time integrator
    dynaModel->setTimeIntegrator(timeIntegrator);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setPointSize(10.);
    material->setLineWidth(4.);
    material->setEdgeColor(Color::Color::Orange);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Scene object 1: Dragon
    auto deformableObj = std::make_shared<FeDeformableObject>("Dragon");
    deformableObj->addVisualModel(surfMeshModel);

    deformableObj->setPhysicsGeometry(tetMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>(tetMesh, surfMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map
    deformableObj->setDynamicalModel(dynaModel);
    scene->addSceneObject(deformableObj);

    return deformableObj;
}
