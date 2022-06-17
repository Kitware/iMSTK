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

#include "imstkPointSet.h"
#include "imstkRenderTest.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkVecDataArray.h"
#include "imstkVertexLabelVisualModel.h"

TEST_F(VisualTest, VertexLabel)
{
    auto pointSet = std::make_shared<PointSet>();

    VecDataArray<double, 3> vertices(4);
    vertices[0] = Vec3d(-0.5, 0.0, -0.5);
    vertices[1] = Vec3d(0.5, 0.0, -0.5);
    vertices[2] = Vec3d(0.0, 0.0, 0.75);
    vertices[3] = Vec3d(0.0, 0.5, 0.0);

    pointSet->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices));

    auto visualModel = std::make_shared<VertexLabelVisualModel>();
    visualModel->setGeometry(pointSet);

    auto sceneObject = std::make_shared<SceneObject>();
    sceneObject->addVisualModel(visualModel);

    m_scene = std::make_shared<Scene>("VertexLabelScene");
    m_scene->addSceneObject(sceneObject);

    runFor(1.0);
}