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
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkDirectionalLight.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief This test is used to investigate Triangle Vs Triangle collision
/// of the SurfaceMeshToSurfaceMeshCD method.
/// It displays the collision data, and allows users to investigate various cases
/// by moving the geometry around with keyboard controls i,j,k,l,o,u
///
TEST_F(CollisionDetectionVisualTest, TriangleVsTriangle)
{
    // Setup the scene
    m_scene = std::make_shared<Scene>("TriangleVsTriangleTest");
    m_scene->getActiveCamera()->setPosition(0.18, 1.08, 1.34);
    m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    m_scene->getActiveCamera()->setViewUp(0.011, 0.78, -0.63);

    auto                    obj1 = std::make_shared<CollidingObject>("obj1");
    auto                    triangleMesh1 = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices1(3);
    triangleVertices1[0] = Vec3d(0.1, 0.0, -0.5);
    triangleVertices1[1] = Vec3d(0.1, 0.0, 0.5);
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

    auto                    obj2 = std::make_shared<CollidingObject>("obj2");
    auto                    triangleMesh2 = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices2(3);
    triangleVertices2[0] = Vec3d(-0.1, 0.5, 0.0);
    triangleVertices2[1] = Vec3d(-0.1, -0.5, 0.0);
    triangleVertices2[2] = Vec3d(0.5, 0.0, 0.0);
    VecDataArray<int, 3> triangleIndices2(1);
    triangleIndices2[0] = Vec3i(0, 1, 2);
    triangleMesh2->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices2),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices2));
    obj2->setVisualGeometry(triangleMesh2);
    obj2->setCollidingGeometry(triangleMesh2);
    obj2->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    m_scene->addSceneObject(obj2);

    auto cd = std::make_shared<SurfaceMeshToSurfaceMeshCD>();
    cd->setInputGeometryA(triangleMesh1);
    cd->setInputGeometryB(triangleMesh2);
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

    m_keyPressFunc =
        [&](KeyEvent* e)
        {
            const double s = 0.05;
            if (e->m_key == 'i')
            {
                triangleMesh2->translate(Vec3d(0.0, 0.0, 1.0) * s);
            }
            else if (e->m_key == 'k')
            {
                triangleMesh2->translate(Vec3d(0.0, 0.0, -1.0) * s);
            }
            else if (e->m_key == 'j')
            {
                triangleMesh2->translate(Vec3d(-1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'l')
            {
                triangleMesh2->translate(Vec3d(1.0, 0.0, 0.0) * s);
            }
            else if (e->m_key == 'u')
            {
                triangleMesh2->translate(Vec3d(0.0, -1.0, 0.0) * s);
            }
            else if (e->m_key == 'o')
            {
                triangleMesh2->translate(Vec3d(0.0, 1.0, 0.0) * s);
            }
            triangleMesh2->postModified();
            triangleMesh2->updatePostTransformData();
            cd->update();
            cdDebugObj->debugUpdate();
        };
    m_startingFunc =
        [&](Event*)
        {
            cdDebugObj->debugUpdate();
        };

    runFor(2.0);
}