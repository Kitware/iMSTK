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

#include "imstkCollisionDetectionVisualTest.h"
#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDataDebugObject.h"
#include "imstkGeometryUtilities.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkOrientedBox.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief This test is used to investigate Triangle Vs Triangle collision
/// of the MeshToMeshBruteForceCD method.
/// It displays the collision data, and allows users to investigate various cases
/// by moving the geometry around with keyboard controls i,j,k,l,o,u
///
TEST_F(CollisionDetectionVisualTest, BoxVsBoxSurfaceMesh)
{
    // Create a box mesh
    auto box1 = std::make_shared<OrientedBox>(
        Vec3d::Zero(), Vec3d(0.5, 0.5, 0.5), Quatd::Identity());
    auto box2 = std::make_shared<OrientedBox>(
        Vec3d::Zero(), Vec3d(0.4, 0.4, 0.4), Quatd::Identity());

    std::shared_ptr<SurfaceMesh> box1Mesh = GeometryUtils::toSurfaceMesh(box1);
    std::shared_ptr<SurfaceMesh> box2Mesh = GeometryUtils::toSurfaceMesh(box2);
    box2Mesh->rotate(Vec3d(0.0, 0.0, 1.0), PI_2 * 0.5);
    box2Mesh->rotate(Vec3d(1.0, 0.0, 0.0), PI_2 * 0.5);
    box2Mesh->translate(Vec3d(0.0, 0.8, 0.8));

    // Setup the scene
    m_scene = std::make_shared<Scene>("BoxBoxMeshTest");
    m_scene->getActiveCamera()->setPosition(0.073, 1.743, 3.679);
    m_scene->getActiveCamera()->setFocalPoint(0.333, 0.333, 0.25);
    m_scene->getActiveCamera()->setViewUp(0.041, 0.928, -0.371);

    auto obj1 = std::make_shared<CollidingObject>("obj1");
    obj1->setVisualGeometry(box1Mesh);
    obj1->setCollidingGeometry(box1Mesh);
    obj1->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
    obj1->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    m_scene->addSceneObject(obj1);

    auto obj2 = std::make_shared<CollidingObject>("obj2");
    obj2->setVisualGeometry(box2Mesh);
    obj2->setCollidingGeometry(box2Mesh);
    obj2->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
    obj2->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    m_scene->addSceneObject(obj2);

    auto cd = std::make_shared<MeshToMeshBruteForceCD>();
    cd->setInputGeometryA(box1Mesh);
    cd->setInputGeometryB(box2Mesh);
    cd->setGenerateEdgeEdgeContacts(true);
    cd->update();

    // Debug Collision Geometry
    auto cdDebugObj = std::make_shared<CollisionDataDebugObject>();
    cdDebugObj->setInputCD(cd->getCollisionData());
    cdDebugObj->setPrintContacts(true);
    m_scene->addSceneObject(cdDebugObj);

    std::cout << "================================================\n";
    std::cout << "Key i/j/k/u/o move the cube\n";
    std::cout << "Key 1/2/3/4 rotate the cube\n";
    std::cout << "================================================\n\n";

    m_keyPressFunc =
        [&](KeyEvent* e)
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
            // Immediately update the geometry, recompute the CD, update CD debug geometry
            box2Mesh->postModified();
            box2Mesh->updatePostTransformData();
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