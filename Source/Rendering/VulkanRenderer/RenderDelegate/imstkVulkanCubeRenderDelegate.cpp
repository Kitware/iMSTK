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

#include "imstkVulkanCubeRenderDelegate.h"

namespace imstk
{
VulkanCubeRenderDelegate::VulkanCubeRenderDelegate(std::shared_ptr<Cube> cube, VulkanMemoryManager& memoryManager)
    : m_geometry(cube)
{
    auto width = m_geometry->getWidth();

    auto source = vtkSmartPointer<vtkCubeSource>::New();
    source->SetCenter(WORLD_ORIGIN[0], WORLD_ORIGIN[1], WORLD_ORIGIN[2]);
    source->SetXLength(width);
    source->SetYLength(width);
    source->SetZLength(width);
    source->Update();

    auto triangulate = vtkSmartPointer<vtkTriangleFilter>::New();
    triangulate->SetInputConnection(source->GetOutputPort());

    triangulate->Update();

    auto sourceData = triangulate->GetOutput();

    auto positions = sourceData->GetPoints();
    auto normals = sourceData->GetPointData()->GetNormals();
    auto triangles = sourceData->GetPolys();

    for (int i = 0; i < sourceData->GetNumberOfPoints(); i++)
    {
        auto position = positions->GetPoint(i);
        auto normal = normals->GetTuple(i);

        VulkanBasicVertex cubeVertex;

        cubeVertex.position[0] = position[0];
        cubeVertex.position[1] = position[1];
        cubeVertex.position[2] = position[2];

        cubeVertex.normal[0] = normal[0];
        cubeVertex.normal[1] = normal[1];
        cubeVertex.normal[2] = normal[2];

        m_cubeVertices.push_back(cubeVertex);
    }

    triangles->InitTraversal();

    for (int i = 0; i < triangles->GetNumberOfCells(); i++)
    {
        auto points = vtkSmartPointer<vtkIdList>::New();
        auto triangle = triangles->GetNextCell(points);

        std::array<uint32_t, 3> trianglePoints = {
            (uint32_t)points->GetId(0),
            (uint32_t)points->GetId(1),
            (uint32_t)points->GetId(2)
        };

        m_cubeTriangles.push_back(trianglePoints);
    }

    m_numVertices = (uint32_t)m_cubeVertices.size();
    m_numTriangles = (uint32_t)m_cubeTriangles.size();
    m_vertexSize = sizeof(VulkanBasicVertex);

    this->initializeData(memoryManager);

    m_vertexBuffer->updateVertexBuffer(&m_cubeVertices, &m_cubeTriangles);

    this->update();
}

void
VulkanCubeRenderDelegate::update()
{
    this->updateTransform(m_geometry);
    m_vertexUniformBuffer->updateUniforms(sizeof(VulkanLocalVertexUniforms),
        (void *)&m_localVertexUniforms);
}

std::shared_ptr<Geometry>
VulkanCubeRenderDelegate::getGeometry() const
{
    return m_geometry;
}
}