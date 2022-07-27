/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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