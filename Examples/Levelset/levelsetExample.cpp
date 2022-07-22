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
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVTKViewer.h"

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

using namespace imstk;

///
/// \brief Creates a shrinking level set dragon
///
std::shared_ptr<LevelSetDeformableObject>
makeLevelsetObj(const std::string& name)
{
    imstkNew<LevelSetDeformableObject> levelsetObj(name);

    // Setup the Geometry (read dragon mesh)
    auto dragonSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    //dragonSurfMesh->translate(position, Geometry::TransformType::ApplyToData);
    imstkNew<SurfaceMeshDistanceTransform> computeSdf;
    computeSdf->setInputMesh(dragonSurfMesh);
    computeSdf->setDimensions(50, 50, 50);
    computeSdf->update();
    std::shared_ptr<ImageData> initLvlsetImage = computeSdf->getOutputImage();

    // Setup the Parameters
    imstkNew<LevelSetModelConfig> lvlsetConfig;
    lvlsetConfig->m_sparseUpdate = false;
    lvlsetConfig->m_dt = 0.003;
    lvlsetConfig->m_constantVelocity = -1.0;

    // Setup the Model
    imstkNew<LevelSetModel> dynamicalModel;
    dynamicalModel->setModelGeometry(initLvlsetImage);
    dynamicalModel->configure(lvlsetConfig);

    // Setup the VisualModel
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(initLvlsetImage);
    imstkNew<VolumeRenderMaterial> mat;
    {
        vtkNew<vtkColorTransferFunction> color;
        color->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
        color->AddRGBPoint(-0.01, 0.0, 0.0, 1.0);
        mat->getVolumeProperty()->SetColor(color);
        vtkNew<vtkPiecewiseFunction> opacity;
        opacity->AddPoint(0.0, 0.0);
        opacity->AddPoint(-0.01, 1.0);
        mat->getVolumeProperty()->SetScalarOpacity(opacity);
        mat->getVolumeProperty()->SetInterpolationTypeToLinear();
        mat->getVolumeProperty()->ShadeOn();
    }
    visualModel->setRenderMaterial(mat);

    // Setup the Object
    levelsetObj->addVisualModel(visualModel);
    levelsetObj->setPhysicsGeometry(initLvlsetImage);
    levelsetObj->setDynamicalModel(dynamicalModel);

    return levelsetObj;
}

///
/// \brief This example demonstrates evolution of a levelset
///
int
main()
{
    // Log to file and stdout
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("LevelsetScene");
    {
        std::shared_ptr<LevelSetDeformableObject> obj = makeLevelsetObj("DragonLevelset");
        scene->addSceneObject(obj);

        // Light (white)
        imstkNew<DirectionalLight> whiteLight;
        whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        whiteLight->setIntensity(1.0);
        scene->addLight("whitelight", whiteLight);

        // Adjust camera
        scene->getActiveCamera()->setPosition(0.0, 10.0, -10.0);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}
