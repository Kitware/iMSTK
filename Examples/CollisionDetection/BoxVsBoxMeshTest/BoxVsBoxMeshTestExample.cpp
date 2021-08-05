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
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This examples is used to demonstrate Triangle Vs Triangle collision
/// of the MeshToMeshBruteForceCD method.
/// It displays the collision data, and allows users to investigate various cases
/// by moving the geometry around with keyboard controls i,j,k,l,o,u
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create a box mesh
    imstkNew<OrientedBox> box1(Vec3d::Zero(), Vec3d(0.5, 0.5, 0.5), Quatd::Identity());
    imstkNew<OrientedBox> box2(Vec3d::Zero(), Vec3d(0.4, 0.4, 0.4), Quatd::Identity());

    std::shared_ptr<SurfaceMesh> box1Mesh = GeometryUtils::toSurfaceMesh(box1);
    std::shared_ptr<SurfaceMesh> box2Mesh = GeometryUtils::toSurfaceMesh(box2);
    box2Mesh->rotate(Vec3d(0.0, 0.0, 1.0), PI_2 * 0.5);
    box2Mesh->rotate(Vec3d(1.0, 0.0, 0.0), PI_2 * 0.5);
    box2Mesh->translate(Vec3d(0.0, 0.8, 0.8));

    // Setup the scene
    imstkNew<Scene> scene("BoxBoxMeshTest");
    scene->getActiveCamera()->setPosition(0.073, 1.743, 3.679);
    scene->getActiveCamera()->setFocalPoint(0.333, 0.333, 0.25);
    scene->getActiveCamera()->setViewUp(0.041, 0.928, -0.371);

    imstkNew<CollidingObject> obj1("obj1");
    obj1->setVisualGeometry(box1Mesh);
    obj1->setCollidingGeometry(box1Mesh);
    obj1->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
    obj1->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    scene->addSceneObject(obj1);

    imstkNew<CollidingObject> obj2("obj2");
    obj2->setVisualGeometry(box2Mesh);
    obj2->setCollidingGeometry(box2Mesh);
    obj2->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
    obj2->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    scene->addSceneObject(obj2);

    imstkNew<MeshToMeshBruteForceCD> cd;
    cd->setInputGeometryA(box1Mesh);
    cd->setInputGeometryB(box2Mesh);
    cd->setGenerateEdgeEdgeContacts(true);
    cd->update();

    // Debug Collision Geometry
    imstkNew<CollisionDataDebugObject> cdDebugObj;
    cdDebugObj->setInputCD(cd->getCollisionData());
    cdDebugObj->setPrintContacts(true);
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

        cdDebugObj->debugUpdate();

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress, [&](KeyEvent* e)
        {
            const double s = 0.05;
            if (e->m_key == 'i')
            {
                box2Mesh->translate(Vec3d(0.0, 0.0, 1.0) * s);
            }
            else if (e->m_key == 'k')
            {
                box2Mesh->translate(Vec3d(0.0, 0.0, -1.0) * s);
            }
            else if (e->m_key == 'j')
            {
                box2Mesh->translate(Vec3d(-1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'l')
            {
                box2Mesh->translate(Vec3d(1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'u')
            {
                box2Mesh->translate(Vec3d(0.0, -1.0, 0.0) * s);
            }
            else if (e->m_key == 'o')
            {
                box2Mesh->translate(Vec3d(0.0, 1.0, 0.0) * s);
            }
            else if (e->m_key == '1')
            {
                box2Mesh->rotate(Vec3d(0.0, 0.0, 1.0), 0.1);
            }
            else if (e->m_key == '2')
            {
                box2Mesh->rotate(Vec3d(0.0, 0.0, 1.0), -0.1);
            }
            else if (e->m_key == '3')
            {
                box2Mesh->rotate(Vec3d(0.0, 1.0, 0.0), 0.1);
            }
            else if (e->m_key == '4')
            {
                box2Mesh->rotate(Vec3d(0.0, 1.0, 0.0), -0.1);
            }
            box2Mesh->postModified();
            box2Mesh->updatePostTransformData();
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
