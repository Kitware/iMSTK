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
#include "imstkEventObject.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkRenderTest.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVTKViewer.h"

using namespace imstk;

TEST_F(RenderTest, meshFile)
{
    geom = MeshIO::read(iMSTK_DATA_ROOT "/textured_organs/heart.obj");
    geom->scale(0.15, Geometry::TransformType::ConcatenateToTransform);

    createScene();
    runAllMaterials();
}

TEST_F(RenderTest, meshColorFunctionVertices)
{
    VecDataArray<double, 3> points;
    auto                    scalars = std::make_shared<DataArray<float>>();

    for (int i = 0; i < 6; ++i)
    {
        points.push_back({ 0.0, 0.0, static_cast<double>(i) });
        scalars->push_back(i);
        points.push_back({ 1.0, 0.0, static_cast<double>(i) });
        scalars->push_back(i);
    }

    auto mesh = std::make_shared<SurfaceMesh>();
    geom = mesh;

    VecDataArray<int, 3> tris;
    for (int i = 0; i < 5; ++i)
    {
        int j = i * 2;
        tris.push_back({ j + 2, j + 1, j });
        tris.push_back({ j + 3, j + 1, j + 2 });
    }

    mesh->initialize(std::make_shared<VecDataArray<double, 3>>(points),
        std::make_shared<VecDataArray<int, 3>>(tris));
    mesh->setVertexAttribute("scalars", scalars);
    mesh->setVertexScalars("scalars");

    std::shared_ptr<AbstractDataArray> abstracScalars = scalars;

    float val = 0.0f;
    connect<Event>(m_viewer, &VTKViewer::preUpdate,
        [&](Event*)
        {
            if (val < 6.0f)
            {
                val += 0.05f;
            }
            else
            {
                val = 0.0f;
            }
            (*scalars)[0] = val;
            (*scalars)[1] = val;
            (*scalars)[2] = val;
            (*scalars)[3] = val;
            abstracScalars->postModified();
        });

    connect<Event>(m_driver, &SimulationManager::starting,
        [&](Event*)
        {
            m_scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
            m_scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));
        });

    createScene();
    applyColor();
    runFor(2.0);
}

TEST_F(RenderTest, meshColorFunctionCells)
{
    VecDataArray<double, 3> points;
    auto                    scalars = std::make_shared<DataArray<float>>();

    for (int i = 0; i < 6; ++i)
    {
        points.push_back({ 0.0, 0.0, static_cast<double>(i) });
        points.push_back({ 1.0, 0.0, static_cast<double>(i) });
    }

    VecDataArray<int, 3> tris;
    for (int i = 0; i < 5; ++i)
    {
        int j = i * 2;
        tris.push_back({ j + 2, j + 1, j });
        scalars->push_back(i);
        tris.push_back({ j + 3, j + 1, j + 2 });
        scalars->push_back(i);
    }

    auto mesh = std::make_shared<SurfaceMesh>();
    mesh->initialize(std::make_shared<VecDataArray<double, 3>>(points),
        std::make_shared<VecDataArray<int, 3>>(tris));
    mesh->setCellAttribute("scalars", scalars);
    mesh->setCellScalars("scalars");
    geom = mesh;

    float val = 0.0f;
    connect<Event>(m_viewer, VTKViewer::preUpdate,
        [&](Event*)
        {
            if (val < 6.0f)
            {
                val += 0.05f;
            }
            else
            {
                val = 0.0f;
            }
            (*scalars)[0] = val;
            (*scalars)[1] = val;
            (*scalars)[2] = val;
            (*scalars)[3] = val;
            scalars->postModified();
        });

    connect<Event>(m_driver, &SimulationManager::starting,
        [&](Event*)
        {
            m_scene->getActiveCamera()->setPosition(Vec3d(0.0, 12.0, 3.0));
            m_scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.0, 3.01));
        });

    createScene();
    applyColor();
    runFor(2.0);
}

TEST_F(RenderTest, meshColorFunctionDynamicVertices)
{
    auto mesh    = std::make_shared<SurfaceMesh>();
    auto points  = std::make_shared<VecDataArray<double, 3>>();
    auto tris    = std::make_shared<VecDataArray<int, 3>>();
    auto scalars = std::make_shared<DataArray<float>>();
    mesh->initialize(points, tris);
    geom = mesh;

    double scale = 1.0;
    connect<Event>(m_viewer, VTKViewer::preUpdate,
        [&](Event*)
        {
            scale       += 0.01;
            auto points  = std::make_shared<VecDataArray<double, 3>>();
            auto tris    = std::make_shared<VecDataArray<int, 3>>();
            auto scalars = std::make_shared<DataArray<float>>();
            mesh->clear();
            for (int i = 0; i < 6; ++i)
            {
                points->push_back({ 0.0, 0.0, static_cast<double>(i) });
                scalars->push_back(static_cast<float>(i));
                points->push_back({ 1.0 * scale, 0.0, static_cast<double>(i) });
                scalars->push_back(i);
            }

            for (int i = 0; i < 5; ++i)
            {
                int j = i * 2;
                tris->push_back({ j + 2, j + 1, j });
                tris->push_back({ j + 3, j + 1, j + 2 });
            }

            mesh->initialize(points, tris);
            mesh->setVertexAttribute("scalars", scalars);
            mesh->setVertexScalars("scalars");
            mesh->computeVertexNormals();
            mesh->postModified();
        });

    connect<Event>(m_driver, &SimulationManager::starting,
        [&](Event*)
        {
            m_scene->getActiveCamera()->setPosition(Vec3d(0.0, 12.0, 3.0));
            m_scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.0, 3.01));
        });

    createScene();
    applyColor();
    runFor(2.0);
}

TEST_F(RenderTest, meshColorFunctionDynamicCells)
{
    auto mesh    = std::make_shared<SurfaceMesh>();
    auto points  = std::make_shared<VecDataArray<double, 3>>();
    auto tris    = std::make_shared<VecDataArray<int, 3>>();
    auto scalars = std::make_shared<DataArray<float>>();
    mesh->initialize(points, tris);
    geom = mesh;

    double scale = 1.0;
    connect<Event>(m_viewer, &VTKViewer::preUpdate,
        [&](Event*)
        {
            scale       += 0.01;
            auto points  = std::make_shared<VecDataArray<double, 3>>();
            auto tris    = std::make_shared<VecDataArray<int, 3>>();
            auto scalars = std::make_shared<DataArray<float>>();
            mesh->clear();
            for (int i = 0; i < 6; ++i)
            {
                points->push_back({ 0.0, 0.0, static_cast<double>(i) });
                points->push_back({ 1.0 * scale, 0.0, static_cast<double>(i) });
            }

            for (int i = 0; i < 5; ++i)
            {
                int j = i * 2;
                tris->push_back({ j + 2, j + 1, j });
                scalars->push_back(i);
                tris->push_back({ j + 3, j + 1, j + 2 });
                scalars->push_back(i);
            }

            mesh->initialize(points, tris);
            mesh->setCellAttribute("scalars", scalars);
            mesh->setCellScalars("scalars");
            mesh->computeVertexNormals();
            mesh->postModified();
        });

    connect<Event>(m_driver, &SimulationManager::starting,
        [&](Event*)
        {
            m_scene->getActiveCamera()->setPosition(Vec3d(0.0, 12.0, 3.0));
            m_scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.0, 3.01));
        });

    createScene();
    applyColor();
    runFor(2.0);
}