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

#include "imstkRenderTest.h"

#include "imstkEventObject.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

class MeshFileRenderTest : public RenderTest
{
public:

    void createGeometry() override
    {
        geom = MeshIO::read(iMSTK_DATA_ROOT "/textured_organs/heart.obj");
        geom->scale(0.15, Geometry::TransformType::ConcatenateToTransform);
    }
};
TEST_F(MeshFileRenderTest, meshFile)
{
  runAllMaterials();
}

class MeshColorFunctionVerticesRenderTest : public RenderTest
{
public:

    float val = 0.0;
    void createGeometry() override
    {
        imstk::VecDataArray<double, 3> points;
        auto                           scalars = std::make_shared<imstk::DataArray<float>>();

        for (int i = 0; i < 6; ++i)
        {
            points.push_back({ 0, 0, static_cast<double>(i) });
            scalars->push_back(i);
            points.push_back({ 1, 0, static_cast<double>(i) });
            scalars->push_back(i);
        }

        auto mesh = std::make_shared<imstk::SurfaceMesh>();
        geom = mesh;

        imstk::VecDataArray<int, 3> tris;
        for (int i = 0; i < 5; ++i)
        {
            int j = i * 2;
            tris.push_back({ j + 2, j + 1, j });
            tris.push_back({ j + 3, j + 1, j + 2 });
        }

        mesh->initialize(std::make_shared<VecDataArray<double, 3>>(points), std::make_shared<VecDataArray<int, 3>>(tris));
        mesh->setVertexAttribute("scalars", scalars);
        mesh->setVertexScalars("scalars");

        std::shared_ptr<imstk::AbstractDataArray> abstracScalars = scalars;

        auto onPreUpdate = [this, scalars, abstracScalars](Event*)
                           {
                               if (val < 6.0)
                               {
                                   val += 0.05;
                               }
                               else
                               {
                                   val = 0.0;
                               }
                               (*scalars)[0] = val;
                               (*scalars)[1] = val;
                               (*scalars)[2] = val;
                               (*scalars)[3] = val;
                               abstracScalars->postModified();
                           };

        connect<Event>(viewer, VTKViewer::preUpdate, onPreUpdate);
        onPreUpdate(nullptr);
        applyColor();

        scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
        scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));
    }
};
TEST_F(MeshColorFunctionVerticesRenderTest, meshColorFunctionVertices)
{
    runFor(2);
}

class MeshColorFunctionCellsRenderTest : public RenderTest
{
public:

    float val = 0.0;
    void createGeometry() override
    {
        imstk::VecDataArray<double, 3> points;
        auto                           scalars = std::make_shared<imstk::DataArray<float>>();

        for (int i = 0; i < 6; ++i)
        {
            points.push_back({ 0, 0, static_cast<double>(i) });
            points.push_back({ 1, 0, static_cast<double>(i) });
        }

        imstk::VecDataArray<int, 3> tris;
        for (int i = 0; i < 5; ++i)
        {
            int j = i * 2;
            tris.push_back({ j + 2, j + 1, j });
            scalars->push_back(i);
            tris.push_back({ j + 3, j + 1, j + 2 });
            scalars->push_back(i);
        }

        auto mesh = std::make_shared<imstk::SurfaceMesh>();
        mesh->initialize(std::make_shared<VecDataArray<double, 3>>(points), std::make_shared<VecDataArray<int, 3>>(tris));
        mesh->setCellAttribute("scalars", scalars);
        mesh->setCellScalars("scalars");
        geom = mesh;

        auto onPreUpdate = [this, scalars](Event*)
                           {
                               if (val < 6.0)
                               {
                                   val += 0.05;
                               }
                               else
                               {
                                   val = 0.0;
                               }
                               (*scalars)[0] = val;
                               (*scalars)[1] = val;
                               (*scalars)[2] = val;
                               (*scalars)[3] = val;
                               scalars->postModified();
                           };

        connect<Event>(viewer, VTKViewer::preUpdate, onPreUpdate);
        onPreUpdate(nullptr);
        applyColor();

        scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
        scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));
    }
};
TEST_F(MeshColorFunctionCellsRenderTest, meshColorFunctionCells)
{
  runFor(2);
}

class MeshColorFunctionDynamicVerticesRenderTest : public RenderTest
{
public:

    double scale = 1.0;
    void createGeometry() override
    {
        auto mesh    = std::make_shared<imstk::SurfaceMesh>();
        auto points  = std::make_shared<imstk::VecDataArray<double, 3>>();
        auto tris    = std::make_shared<imstk::VecDataArray<int, 3>>();
        auto scalars = std::make_shared<imstk::DataArray<float>>();
        mesh->initialize(points, tris);
        geom = mesh;

        auto updateMesh = [this, mesh](Event*)
                          {
                              scale += 0.01;
                              auto points  = std::make_shared<imstk::VecDataArray<double, 3>>();
                              auto tris    = std::make_shared<imstk::VecDataArray<int, 3>>();
                              auto scalars = std::make_shared<imstk::DataArray<float>>();
                              mesh->clear();
                              for (int i = 0; i < 6; ++i)
                              {
                                  points->push_back({ 0, 0, static_cast<double>(i) });
                                  scalars->push_back(i);
                                  points->push_back({ 1 * scale, 0, static_cast<double>(i) });
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
                          };

        connect<Event>(viewer, VTKViewer::preUpdate, updateMesh);
        updateMesh(nullptr);
        applyColor();

        scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
        scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));
    }
};
TEST_F(MeshColorFunctionDynamicVerticesRenderTest, meshColorFunctionDynamicVertices)
{
    runFor(2);
}

class MeshColorFunctionDynamicCellsRenderTest : public RenderTest
{
public:

    double scale = 1.0;
    void createGeometry() override
    {
        auto mesh    = std::make_shared<imstk::SurfaceMesh>();
        auto points  = std::make_shared<imstk::VecDataArray<double, 3>>();
        auto tris    = std::make_shared<imstk::VecDataArray<int, 3>>();
        auto scalars = std::make_shared<imstk::DataArray<float>>();
        mesh->initialize(points, tris);
        geom = mesh;

        auto updateMesh = [this, mesh](Event*)
                          {
                              scale += 0.01;
                              auto points  = std::make_shared<imstk::VecDataArray<double, 3>>();
                              auto tris    = std::make_shared<imstk::VecDataArray<int, 3>>();
                              auto scalars = std::make_shared<imstk::DataArray<float>>();
                              mesh->clear();
                              for (int i = 0; i < 6; ++i)
                              {
                                  points->push_back({ 0, 0, static_cast<double>(i) });
                                  points->push_back({ 1 * scale, 0, static_cast<double>(i) });
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
                          };

        connect<Event>(viewer, viewer->preUpdate, updateMesh);
        updateMesh(nullptr);
        applyColor();

        scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
        scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));
    }
};
TEST_F(MeshColorFunctionDynamicCellsRenderTest, meshColorFunctionDynamicCells)
{
    runFor(2);
}
