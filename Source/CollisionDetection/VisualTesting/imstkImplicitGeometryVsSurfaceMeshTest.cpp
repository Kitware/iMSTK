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
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkPlane.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

using namespace imstk;

///
/// \brief This test is used to investigate ImplicitGeometry Vs SurfaceMesh collision
/// of the ImplicitGeometryToPointSet method.
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, ImplicitGeometryVsSurfaceMesh)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0.18, 1.08, 1.34);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0.011, 0.78, -0.63);

    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, 0.0, 0.0);
    implicitGeom->setWidth(0.5);
    m_cdGeom1 = implicitGeom;

    auto                    triangleMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices(3);
    triangleVertices[0] = Vec3d(0.1, -0.5, 0.0);
    triangleVertices[1] = Vec3d(0.1, 0.5, 0.0);
    triangleVertices[2] = Vec3d(-0.5, 0.0, 0.0);
    VecDataArray<int, 3> triangleIndices(1);
    triangleIndices[0] = Vec3i(0, 1, 2);
    triangleMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices));
    m_cdGeom2 = triangleMesh;

    auto cd = std::make_shared<ImplicitGeometryToPointSetCD>();
    cd->setGenerateCD(true, true);
    cd->setInputGeometryA(triangleMesh);
    cd->setInputGeometryB(implicitGeom);
    cd->update();
    m_collisionMethod = cd;

    createScene();
    runFor(2.0);
}