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
#include "imstkSurfaceMesh.h"
#include "imstkCamera.h"
#include "imstkPlane.h"
#include "imstkScene.h"

#include <array>
#include <string>

using namespace imstk;

///
/// \brief Create a PbdObject and add it to a \p scene
///
std::shared_ptr<PbdObject> createAndAddPbdObject(std::shared_ptr<Scene> scene,
                                                 const std::string&     tetMeshName);

// mesh file names
const std::string& tetMeshFileName = iMSTK_DATA_ROOT "textured_organs/heart_volume.vtk";

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

    // create and add a PBD object
    createAndAddPbdObject(scene, tetMeshFileName);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1.1);
    scene->addLight(light);

    simManager->setActiveScene(scene);
    simManager->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    simManager->start(SimulationStatus::Paused);

    return 0;
}

std::shared_ptr<PbdObject>
createAndAddPbdObject(std::shared_ptr<Scene> scene,
                      const std::string&     tetMeshName)
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(tetMeshName));
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.3, Geometry::TransformType::ApplyToData);
    auto surfMesh = std::make_shared<SurfaceMesh>();
    tetMesh->extractSurfaceMesh(surfMesh, true);

    auto map = std::make_shared<TetraTriangleMap>(tetMesh, surfMesh);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setColor(Color(220. / 255.0, 100. / 255.0, 70. / 255.0));
    material->setMetalness(100.9f);
    material->setRoughness(0.5);
    material->setEdgeColor(Color::Teal);
    material->setAmbientLightCoeff(50.);
    material->setShadingModel(RenderMaterial::ShadingModel::Phong);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    auto deformableObj = std::make_shared<PbdObject>("DeformableObject");
    auto pbdModel      = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tetMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // FEM constraint
    pbdParams->m_femParams->m_YoungModulus = 500.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 75, 82, 84, 94, 95, 105, 110, 124, 139, 150, 161, 171, 350 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_iterations = 6;

    // Set the parameters
    pbdModel->configure(pbdParams);
    pbdModel->setDefaultTimeStep(0.02);
    pbdModel->setTimeStepSizeType(imstk::TimeSteppingType::Fixed);

    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(tetMesh);
    deformableObj->setPhysicsToVisualMap(map); //assign the computed map

    scene->addSceneObject(deformableObj);

    return deformableObj;
}
