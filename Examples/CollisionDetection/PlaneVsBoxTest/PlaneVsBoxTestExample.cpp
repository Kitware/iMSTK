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
#include "imstkCollisionDataDebugObject.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"

using namespace imstk;

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PbdCollisionOneDragon");

    scene->getActiveCamera()->setPosition(0, 3.0, 20.0);
    scene->getActiveCamera()->setFocalPoint(0.0, -10.0, 0.0);

    imstkNew<CollidingObject> obj1("obj1");
    imstkNew<OrientedBox>     cube(Vec3d(0.0, 0.0, 0.0), Vec3d(0.5, 0.5, 0.5), Quatd(Rotd(1.0, Vec3d(0.0, 0.0, 1.0))));
    auto                      surfMesh1 = GeometryUtils::toSurfaceMesh(cube);
    obj1->setVisualGeometry(surfMesh1);
    obj1->setCollidingGeometry(surfMesh1);
    obj1->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    obj1->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    obj1->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::Flat);
    scene->addSceneObject(obj1);

    imstkNew<CollidingObject> obj2("obj2");
    imstkNew<Plane>           plane(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(2.0);
    auto surfMesh2 = GeometryUtils::toSurfaceMesh(plane);
    obj2->setVisualGeometry(surfMesh2);
    obj2->setCollidingGeometry(surfMesh2);
    obj2->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(obj2);

    imstkNew<SurfaceMeshToSurfaceMeshCD> cd;
    cd->setInputGeometryA(surfMesh1);
    cd->setInputGeometryB(surfMesh2);
    cd->update();

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    imstkNew<CollisionDataDebugObject> cdDebugObj;
    cdDebugObj->setInputCD(cd->getCollisionData());
    cdDebugObj->setPrintContacts(true);

    // Debug geometry
    scene->addSceneObject(cdDebugObj);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer())->setAxesLength(0.05, 0.05, 0.05);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);

        sceneManager->init();
        sceneManager->update();

        //cdDebugObj->debugUpdate();

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress, [&](KeyEvent* e)
        {
            const double s = 0.05;
            if (e->m_key == 'i')
            {
                surfMesh1->translate(Vec3d(0.0, 0.0, 1.0) * s);
            }
            else if (e->m_key == 'k')
            {
                surfMesh1->translate(Vec3d(0.0, 0.0, -1.0) * s);
            }
            else if (e->m_key == 'j')
            {
                surfMesh1->translate(Vec3d(-1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'l')
            {
                surfMesh1->translate(Vec3d(1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'u')
            {
                surfMesh1->translate(Vec3d(0.0, -1.0, 0.0) * s);
            }
            else if (e->m_key == 'o')
            {
                surfMesh1->translate(Vec3d(0.0, 1.0, 0.0) * s);
            }
            surfMesh1->postModified();
            cd->update();
            sceneManager->update();
            cdDebugObj->debugUpdate();
        });

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}
