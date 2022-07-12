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
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief This test is used to investigate Triangle Vs Capsule collision
/// of the SurfaceMeshToCapsuleCD method
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, TriangleVsCapsule)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0, 2.74, 2.69);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0, 0.71, -0.71);

    m_cdGeom1 = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);
    m_cdObj1->getVisualModel(0)->getRenderMaterial()->setDisplayMode(
        RenderMaterial::DisplayMode::Surface);

    auto triangleMesh = std::make_shared<SurfaceMesh>();

    VecDataArray<double, 3> triangleVertices(3);
    triangleVertices[0] = Vec3d(-1.33, 0.24, -1.0);
    triangleVertices[1] = Vec3d(1.33, 0.24, -1.0);
    triangleVertices[2] = Vec3d(0.0, 0.24, 1.0);
    VecDataArray<int, 3> triangleIndices(1);
    triangleIndices[0] = Vec3i(0, 1, 2);
    triangleMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices));
    m_cdGeom2 = triangleMesh;

    m_collisionMethod = std::make_shared<SurfaceMeshToCapsuleCD>();
    m_collisionMethod->setInputGeometryA(m_cdGeom2);
    m_collisionMethod->setInputGeometryB(m_cdGeom1);
    m_collisionMethod->update();

    createScene();
    runFor(2.0);
}