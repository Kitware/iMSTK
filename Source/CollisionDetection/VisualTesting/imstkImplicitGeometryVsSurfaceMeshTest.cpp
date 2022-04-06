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
#include "imstkImplicitGeometry.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkVisualTestingUtils.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This test is used to investigate ImplicitGeometry Vs SurfaceMesh collision
/// of the ImplicitGeometryToPointSet method.
/// It displays the collision data, and allows users to investigate various cases
/// by moving the geometry around with keyboard controls i,j,k,l,o,u
///
TEST_F(VisualTestManager, ImplicitGeometryVsSurfaceMesh)
{
    // Setup the scene
    m_scene = std::make_shared<Scene>("ImplicitGeometryVsSurfaceMesh");
    m_scene->getActiveCamera()->setPosition(0.18, 1.08, 1.34);
    m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    m_scene->getActiveCamera()->setViewUp(0.011, 0.78, -0.63);

    auto                    obj1 = std::make_shared<CollidingObject>("obj1");
    auto                    triangleMesh1 = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices1(3);
    triangleVertices1[0] = Vec3d(0.1, -0.5, 0.0);
    triangleVertices1[1] = Vec3d(0.1, 0.5, 0.0);
    triangleVertices1[2] = Vec3d(-0.5, 0.0, 0.0);
    VecDataArray<int, 3> triangleIndices1(1);
    triangleIndices1[0] = Vec3i(0, 1, 2);
    triangleMesh1->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices1),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices1));
    obj1->setVisualGeometry(triangleMesh1);
    obj1->setCollidingGeometry(triangleMesh1);
    obj1->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    m_scene->addSceneObject(obj1);

    auto obj2 = std::make_shared<CollidingObject>("obj2");
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, 0.0, 0.0);
    implicitGeom->setWidth(0.5);
    obj2->setVisualGeometry(implicitGeom);
    obj2->setCollidingGeometry(implicitGeom);
    obj2->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    m_scene->addSceneObject(obj2);

    auto cd = std::make_shared<ImplicitGeometryToPointSetCD>();
    cd->setGenerateCD(true, true);
    cd->setInputGeometryA(triangleMesh1);
    cd->setInputGeometryB(implicitGeom);
    cd->update();

    // Debug geometry to visualize collision data
    auto cdDebugObj = std::make_shared<CollisionDataDebugObject>();
    cdDebugObj->setInputCD(cd->getCollisionData());
    cdDebugObj->setPrintContacts(true);
    m_scene->addSceneObject(cdDebugObj);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    m_scene->addLight("Light", light);

    std::cout << "================================================\n";
    std::cout << "Key i/j/k/u/o move the triangle\n";
    std::cout << "================================================\n\n";

    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
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
            triangleMesh1->updatePostTransformData();
            cd->update();
            cdDebugObj->debugUpdate();
        });
   connect<Event>(m_driver, &SimulationManager::starting,
        [&](Event*)
        {
            cdDebugObj->debugUpdate();
        });

    runFor(2.0);
}