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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This examples is used to demonstrate Triangle Vs Sphere collision
/// of the SurfaceMeshToSphereCD method
/// It displays the collision data, and allows users to investigate various cases
/// by moving the geometry around with keyboard controls i,j,k,l,o,u
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("TriangleVsSphereTest");
    scene->getActiveCamera()->setPosition(0, 2.74, 2.69);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0, 0.71, -0.71);

    imstkNew<CollidingObject>         obj1("obj1");
    imstkNew<SurfaceMesh>             triangleMesh1;
    imstkNew<VecDataArray<double, 3>> triangleVertices1Ptr(3);
    VecDataArray<double, 3>&          triangleVertices1 = *triangleVertices1Ptr.get();
    triangleVertices1[0] = Vec3d(-0.5, 1.1, -0.5);
    triangleVertices1[1] = Vec3d(0.5, 1.1, -0.5);
    triangleVertices1[2] = Vec3d(0.0, 1.1, 0.5);
    imstkNew<VecDataArray<int, 3>> triangleIndices1Ptr(1);
    VecDataArray<int, 3>&          triangleIndices1 = *triangleIndices1Ptr.get();
    triangleIndices1[0] = Vec3i(0, 1, 2);
    triangleMesh1->initialize(triangleVertices1Ptr, triangleIndices1Ptr);
    obj1->setVisualGeometry(triangleMesh1);
    obj1->setCollidingGeometry(triangleMesh1);
    obj1->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(obj1);

    imstkNew<CollidingObject> obj2("obj2");
    imstkNew<Sphere>          sphere(Vec3d(0.0, 0.0, 0.0), 1.0);
    obj2->setVisualGeometry(sphere);
    obj2->setCollidingGeometry(sphere);
    obj2->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
    scene->addSceneObject(obj2);

    imstkNew<SurfaceMeshToSphereCD> cd;
    cd->setInputGeometryA(triangleMesh1);
    cd->setInputGeometryB(sphere);
    cd->update();

    // Debug geometry
    imstkNew<CollisionDataDebugObject> cdDebugObj;
    cdDebugObj->setInputCD(cd->getCollisionData());
    cdDebugObj->setPrintContacts(true);
    scene->addSceneObject(cdDebugObj);

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer())->setAxesLength(0.05, 0.05, 0.05);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->init();
        sceneManager->update();

        cdDebugObj->debugUpdate();

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
        {
            const double s = 0.05;
            if (e->m_key == 'i')
            {
                triangleMesh1->translate(Vec3d(0.0, 0.0, 1.0) * s);
            }
            else if (e->m_key == 'k')
            {
                triangleMesh1->translate(Vec3d(0.0, 0.0, -1.0) * s);
            }
            else if (e->m_key == 'j')
            {
                triangleMesh1->translate(Vec3d(-1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'l')
            {
                triangleMesh1->translate(Vec3d(1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'u')
            {
                triangleMesh1->translate(Vec3d(0.0, -1.0, 0.0) * s);
            }
            else if (e->m_key == 'o')
            {
                triangleMesh1->translate(Vec3d(0.0, 1.0, 0.0) * s);
            }
            triangleMesh1->postModified();
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

        std::cout << "================================================\n";
        std::cout << "Key i/j/k/u/o move the triangle\n";
        std::cout << "================================================\n\n";

        driver->start();
    }

    return 0;
}
