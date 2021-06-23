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
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkMouseSceneControl.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVTKViewer.h"
#include "imstkLineMesh.h"
#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"
#include "imstkLogger.h"

#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkNew.h"
#include "InflatableObject.h"

#ifdef EXAMPLE_USE_HAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkRigidObjectController.h"
#endif

using namespace imstk;

static std::shared_ptr<RigidObject2>
makeToolObj(const std::string& name)
{
    // Setup a Sphere tool
    //imstkNew<Sphere> toolGeometry(Vec3d(0.0, 0.0, 0.0), 1.0);

    imstkNew<LineMesh>                toolGeometry;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 2.0, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    imstkNew<RigidObject2> toolObj(name);
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(10.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 2;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 1000.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos  = Vec3d(0.0, 0.8, 0.0);
    toolObj->getRigidBody()->m_isStatic = true;

    return toolObj;
}

///
/// \brief This example demonstrates the inflatable distance and volume Constraint
/// using Position based dynamics
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBD inflatable object example");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    Vec3d                      tissueSize   = Vec3d(10.0, 3.0, 10.0);
    Vec3i                      tissueDim    = Vec3i(20, 5, 20);
    Vec3d                      tissueCenter = Vec3d(0.1, -1.0, 0.0);
    double                     radius       = tissueSize[0] / 5.0;
    imstkNew<InflatableObject> tissueObj("PBDTissue", tissueSize, tissueDim, tissueCenter);
    scene->addSceneObject(tissueObj);

    // Setup a tool
    Vec3d                         toolTip = tissueCenter + Vec3d(0.0, tissueSize[1] / 2.0, 0.0);
    std::shared_ptr<RigidObject2> toolObj = makeToolObj("RBDTool");
    scene->addSceneObject(toolObj);

    //auto interaction = std::make_shared<PbdRigidObjectCollision>(tissueObj, toolObj, "MeshToMeshBruteForceCD");
    //scene->getCollisionGraph()->addInteraction(interaction);

    // Light
    imstkNew<DirectionalLight> light("Light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        // Add mouse and keyboard controls to the viewer
        imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        viewer->addControl(mouseControl);
        imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        viewer->addControl(keyControl);

#ifdef EXAMPLE_USE_HAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(1.0); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller(toolObj, hapticDeviceClient);
        controller->setTranslationScaling(0.05);
        controller->setLinearKs(1000.0);
        controller->setLinearKd(50.0);
        controller->setAngularKs(10000000.0);
        controller->setAngularKd(500000.0);
        controller->setForceScaling(0.005);
        controller->setSmoothingKernelSize(25);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);
#else
        // Use keyboard controls
        connect<Event>(sceneManager, SceneManager::preUpdate, [&](Event*)
        {
            if (viewer->getKeyboardDevice()->getButton('k') == KEY_PRESS)
            {
                (*toolObj->getRigidBody()->m_pos) += Vec3d(0.0, -0.01, 0.0);
            }
            else if (viewer->getKeyboardDevice()->getButton('i') == KEY_PRESS)
            {
                (*toolObj->getRigidBody()->m_pos) += Vec3d(0.0, 0.01, 0.0);
            }
            else if (viewer->getKeyboardDevice()->getButton('j') == KEY_PRESS)
            {
                (*toolObj->getRigidBody()->m_pos) += Vec3d(-0.01, 0.0, 0.0);
            }
            else if (viewer->getKeyboardDevice()->getButton('l') == KEY_PRESS)
            {
                (*toolObj->getRigidBody()->m_pos) += Vec3d(0.01, 0.0, 0.0);
            }
            else if (viewer->getKeyboardDevice()->getButton('u') == KEY_PRESS)
            {
                (*toolObj->getRigidBody()->m_pos) += Vec3d(0.0, 0.0, -0.01);
            }
            else if (viewer->getKeyboardDevice()->getButton('o') == KEY_PRESS)
            {
                (*toolObj->getRigidBody()->m_pos) += Vec3d(0.0, 0.0, 0.01);
            }
            else if (viewer->getKeyboardDevice()->getButton('s') == KEY_PRESS)
            {
                // The LineMesh used for collision with the PBD tissue
                std::shared_ptr<LineMesh> lineMesh = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
                Vec3d vertice = lineMesh->getVertexPosition(0);

                if ((toolTip - vertice).norm() > 0.01)
                {
                    toolTip = vertice;
                    tissueObj->setUpdateAffectedConstraint();
                }

                tissueObj->inject(toolTip, radius, 0.01);
            }
            else if (viewer->getKeyboardDevice()->getButton('a') == KEY_PRESS)
            {
                // The LineMesh used for collision with the PBD tissue
                std::shared_ptr<LineMesh> lineMesh = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
                Vec3d vertice = lineMesh->getVertexPosition(0);

                if ((toolTip - vertice).norm() > 0.01)
                {
                    toolTip = vertice;
                    tissueObj->setUpdateAffectedConstraint();
                }

                tissueObj->inject(toolTip, radius, -0.01);
            }
            });
#endif

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            // Keep the tool moving in real time
            toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
            //tissueObj->getPbdModel()->getParameters()->m_dt = sceneManager->getDt();
            });

        std::cout << "================================================" << std::endl;
        std::cout << "Key s : injection \n" << "Key a : deflation \n";
        std::cout << "================================================" << std::endl << std::endl;

        driver->start();
    }

    return 0;
}