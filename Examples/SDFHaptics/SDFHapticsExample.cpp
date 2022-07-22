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
#include "imstkCollidingObject.h"
#include "imstkDirectionalLight.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates manually applying point haptics
/// using an SDF
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene>               scene("SDFHaptics");
    std::shared_ptr<SurfaceMesh>  axesMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/axesPoly.vtk");
    std::shared_ptr<ImageData>    sdfImage = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/stanfordBunny/stanfordBunny_SDF.nii");
    imstkNew<SignedDistanceField> sdf(sdfImage->cast(IMSTK_DOUBLE));
    {
        scene->getActiveCamera()->setPosition(-2.3, 23.81, 45.65);
        scene->getActiveCamera()->setFocalPoint(9.41, 8.45, 5.76);

        imstkNew<CollidingObject> bunnyObj("Bunny");
        {
            bunnyObj->setCollidingGeometry(sdf);

            imstkNew<SurfaceMeshFlyingEdges> isoExtract;
            isoExtract->setInputImage(sdfImage);
            isoExtract->update();

            isoExtract->getOutputMesh()->flipNormals();
            bunnyObj->setVisualGeometry(isoExtract->getOutputMesh());

            scene->addSceneObject(bunnyObj);
        }

        imstkNew<SceneObject> axesObj("Axes");
        {
            axesObj->setVisualGeometry(axesMesh);
            scene->addSceneObject(axesObj);
        }

        // Light (white)
        imstkNew<DirectionalLight> whiteLight;
        {
            whiteLight->setDirection(Vec3d(0.0, -8.0, -5.0));
            whiteLight->setIntensity(1.0);
            scene->addLight("whitelight", whiteLight);
        }
    }

    imstkNew<HapticDeviceManager>       hapticManager;
    std::shared_ptr<HapticDeviceClient> client = hapticManager->makeDeviceClient();

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);

        ImplicitFunctionCentralGradient centralGrad;
        centralGrad.setFunction(sdf);
        centralGrad.setDx(sdf->getImage()->getSpacing());
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec3d pos = client->getPosition() * 0.1 + Vec3d(0.0, 0.1, 10.0);

                client->update();
                axesMesh->setTranslation(pos);
                axesMesh->setRotation(client->getOrientation());
                axesMesh->postModified();

                double dx = sdf->getFunctionValue(pos);
                if (dx < 0.0)
                {
                    const Vec3d g = centralGrad(pos);
                    client->setForce(-g.normalized() * dx * 4.0);
                }
            });

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