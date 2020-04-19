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

#include "imstkSimulationManager.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkOneToOneMap.h"
#include "imstkAPIUtilities.h"
#include "imstkTetraTriangleMap.h"
#include "imstkCollisionGraph.h"
#include "imstkSurfaceMesh.h"
#include "imstkCamera.h"
#include "imstkPlane.h"
#include "imstkScene.h"

#include <array>

using namespace imstk;

///
/// \brief This example demonstrates the soft body simulation
/// using Position based dynamics
///
int
main()
{
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("PBDVolume");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj"));
    auto tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg"));
    //auto tetMesh  = TetrahedralMesh::createTetrahedralMeshCover(surfMesh, 10, 6, 6);

    auto map = std::make_shared<TetraTriangleMap>(tetMesh, surfMesh);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    auto deformableObj = std::make_shared<PbdObject>("DeformableObject");
    auto pbdModel      = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tetMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // FEM constraint
    pbdParams->m_YoungModulus = 100.0;
    pbdParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 51, 127, 178 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity = Vec3d(0, -9.8, 0);
    pbdParams->m_maxIter = 45;

    // Set the parameters
    pbdModel->configure(pbdParams);
    pbdModel->setDefaultTimeStep(0.02);
    pbdModel->setTimeStepSizeType(imstk::TimeSteppingType::Fixed);

    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(tetMesh);
    deformableObj->setPhysicsToVisualMap(map); //assign the computed map

    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);

    // Setup plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setTranslation(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    simManager->setActiveScene(scene);
    simManager->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    simManager->start(SimulationStatus::Paused);

    return 0;
}
