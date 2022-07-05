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
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSignedDistanceField.h"
#include "imstkSimulationManager.h"
#include "imstkSphModel.h"
#include "imstkSphObject.h"
#include "imstkSphObjectCollision.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Generate a box-shape of fluid particles
///
std::shared_ptr<VecDataArray<double, 3>>
generateBoxShapeFluid(const double particleRadius, const Vec3d& boxCenter, const Vec3d& boxSize)
{
    double bounds[6] =
    {
        boxCenter.x() - boxSize.x() * 0.5,
        boxCenter.x() + boxSize.x() * 0.5,
        boxCenter.y() - boxSize.y() * 0.5,
        boxCenter.y() + boxSize.y() * 0.5,
        boxCenter.z() - boxSize.z() * 0.5,
        boxCenter.z() + boxSize.z() * 0.5
    };

    const double spacing = 2.0 * particleRadius;
    const Vec3i  dim     = {
        static_cast<int>(boxSize.x() / spacing),
        static_cast<int>(boxSize.y() / spacing),
        static_cast<int>(boxSize.z() / spacing)
    };

    imstkNew<VecDataArray<double, 3>> particles;
    particles->reserve(dim[0] * dim[1] * dim[2]);

    for (double z = bounds[4]; z < bounds[5]; z += spacing)
    {
        for (double y = bounds[2]; y < bounds[3]; y += spacing)
        {
            for (double x = bounds[0]; x < bounds[1]; x += spacing)
            {
                particles->push_back(Vec3d(x, y, z));
            }
        }
    }

    return particles;
}

std::shared_ptr<SphObject>
makeSPHBoxObject(const std::string& name, const double particleRadius, const Vec3d& boxShift, const Vec3d& boxSize)
{
    // Create the sph object
    imstkNew<SphObject> fluidObj(name);

    // Setup the Geometry
    std::shared_ptr<VecDataArray<double, 3>> particles = generateBoxShapeFluid(particleRadius, boxShift, boxSize);
    LOG(INFO) << "Number of particles: " << particles->size();
    imstkNew<PointSet> fluidGeometry;
    fluidGeometry->initialize(particles);

    // Setup the Parameters
    imstkNew<SphModelConfig> sphParams(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    sphParams->m_kernelOverParticleRadiusRatio = 6.0;
    //sphParams->m_viscosityCoeff = 0.5;
    sphParams->m_surfaceTensionStiffness = 5.0;
    sphParams->m_gravity = Vec3d(0.0, -70.0, 0.0);

    // Setup the Model
    imstkNew<SphModel> sphModel;
    sphModel->setModelGeometry(fluidGeometry);
    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Setup the VisualModel
    imstkNew<VisualModel> fluidVisualModel;
    fluidVisualModel->setGeometry(fluidGeometry);
    imstkNew<RenderMaterial> fluidMaterial;
    fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    fluidMaterial->setVertexColor(Color::Orange);
    fluidMaterial->setPointSize(particleRadius);
    fluidVisualModel->setRenderMaterial(fluidMaterial);

    // Setup the Object
    fluidObj->addVisualModel(fluidVisualModel);
    fluidObj->setCollidingGeometry(fluidGeometry);
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->setPhysicsGeometry(fluidGeometry);

    return fluidObj;
}

static std::shared_ptr<CollidingObject>
makeDragonCollidingObject(const std::string& name, const Vec3d& position)
{
    // Create the pbd object
    imstkNew<CollidingObject> collidingObj(name);

    // Setup the Geometry (read dragon mesh)
    auto dragonSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    dragonSurfMesh->translate(position, Geometry::TransformType::ApplyToData);

    imstkNew<SurfaceMeshDistanceTransform> computeSdf;
    computeSdf->setInputMesh(dragonSurfMesh);
    computeSdf->setDimensions(100, 100, 100);
    computeSdf->update();

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    imstkNew<VisualModel> surfMeshModel;
    surfMeshModel->setGeometry(dragonSurfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Setup the Object
    collidingObj->addVisualModel(surfMeshModel);
    collidingObj->setCollidingGeometry(std::make_shared<SignedDistanceField>(computeSdf->getOutputImage()));

    return collidingObj;
}

///
/// \brief This example demonstrates interaction between sph fluid and a
/// colliding object using signed distance fields (SDFs)
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("SPH-Obj-SDFInteraction");
    {
        //scene->getConfig()->taskTimingEnabled = true;
        scene->getActiveCamera()->setPosition(0, 2.0, 15.0);

        // Static Dragon object
        std::shared_ptr<CollidingObject> dragonObj = makeDragonCollidingObject("Dragon", Vec3d(0.0, 0.0, 0.0));
        scene->addSceneObject(dragonObj);

        // SPH fluid box overtop the dragon
        std::shared_ptr<SphObject> sphFluidBox = makeSPHBoxObject("Box", 0.1, Vec3d(0.0, 7.0, 0.0), Vec3d(3.0, 7.0, 3.0));
        scene->addSceneObject(sphFluidBox);

        // Interaction
        scene->addInteraction(
            std::make_shared<SphObjectCollision>(sphFluidBox, dragonObj));

        // Light
        imstkNew<DirectionalLight> light;
        light->setDirection(0.0, -1.0, -1.0);
        light->setIntensity(1);
        scene->addLight("light", light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setBackgroundColors(Color(0.3285, 0.3285, 0.6525), Color(0.13836, 0.13836, 0.2748), true);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.03);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}
