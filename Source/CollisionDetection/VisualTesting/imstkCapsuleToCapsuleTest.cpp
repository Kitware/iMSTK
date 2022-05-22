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
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkRenderMaterial.h"
#include "imstkCapsule.h"
#include "imstkSurfaceMesh.h"
#include "imstkCapsuleToCapsuleCD.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkKeyboardDeviceClient.h"

#include "imstkSphere.h"

using namespace imstk;

///
/// \brief This test is used to investigate Capsule Vs Capsule collision
/// of the CapsuleToCapsuleCD method
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, CapsuleVsCapsule)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0, 2.74, 2.69);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0, 0.71, -0.71);

    m_cdGeom1 = std::make_shared<Capsule>(Vec3d::Zero(), 0.05, 0.2);
    Mat4d test = Mat4d::Identity();
    test(0, 0) = 0.99394; test(0, 1) = 0.109921; test(0, 2) = 0.0; test(0, 3) = -0.000264567;
    test(1, 0) = -0.109921; test(1, 1) = 0.99394; test(1, 2) = 0.0; test(1, 3) = -0.252737;
    m_cdGeom1->setTransform(test);
    m_cdObj1->getVisualModel(0)->getRenderMaterial()->setDisplayMode(
        RenderMaterial::DisplayMode::Surface);

    m_cdGeom2 = std::make_shared<Capsule>(Vec3d(0.01, -0.8, 0.0), 0.2, 0.5);
    m_cdObj2->getVisualModel(0)->getRenderMaterial()->setDisplayMode(
        RenderMaterial::DisplayMode::Surface);

    m_collisionMethod = std::make_shared<CapsuleToCapsuleCD>();
    m_collisionMethod->setInputGeometryA(m_cdGeom2);
    m_collisionMethod->setInputGeometryB(m_cdGeom1);
    m_collisionMethod->update();

    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [this](KeyEvent* e)
        {
            if (e->m_key == '5')
            {
                auto capsule1 = std::dynamic_pointer_cast<Capsule>(m_cdGeom2);
                capsule1->setRadius(capsule1->getRadius() * 1.1);
            }
            else if (e->m_key == '6')
            {
                auto capsule1 = std::dynamic_pointer_cast<Capsule>(m_cdGeom2);
                capsule1->setRadius(capsule1->getRadius() * 0.9);
            }
        });

    createScene();
    runFor(2.0);
}