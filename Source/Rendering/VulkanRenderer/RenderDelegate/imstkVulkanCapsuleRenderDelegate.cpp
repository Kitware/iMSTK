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

#include "imstkVulkanCapsuleRenderDelegate.h"

namespace imstk
{
VulkanCapsuleRenderDelegate::VulkanCapsuleRenderDelegate(std::shared_ptr<Capsule> capsule,
                                                         SceneObject::Type type,
                                                         VulkanMemoryManager& memoryManager)
    : m_geometry(capsule)
{
    auto source = vtkSmartPointer<vtkCapsuleSource>::New();
    source->SetCenter(WORLD_ORIGIN[0], WORLD_ORIGIN[1], WORLD_ORIGIN[2]);
    source->SetRadius(m_geometry->getRadius());
    source->SetCylinderLength(m_geometry->getLength());
    source->SetLatLongTessellation(20);
    source->SetPhiResolution(20);
    source->SetThetaResolution(20);
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

        VulkanBasicVertex capsuleVertex;

        capsuleVertex.position[0] = position[0];
        capsuleVertex.position[1] = position[1];
        capsuleVertex.position[2] = position[2];

        capsuleVertex.normal[0] = normal[0];
        capsuleVertex.normal[1] = normal[1];
        capsuleVertex.normal[2] = normal[2];

        m_capsuleVertices.push_back(capsuleVertex);
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

        m_capsuleTriangles.push_back(trianglePoints);
    }

    m_numVertices = (uint32_t)m_capsuleVertices.size();
    m_numTriangles = (uint32_t)m_capsuleTriangles.size();
    m_vertexSize = sizeof(VulkanBasicVertex);

    if (!m_geometry->getRenderMaterial())
    {
        m_geometry->setRenderMaterial(std::make_shared<RenderMaterial>());
    }

    this->initializeData(memoryManager, m_geometry->getRenderMaterial());

    m_vertexBuffer->updateVertexBuffer(&m_capsuleVertices, &m_capsuleTriangles);

    this->update(0);
}

void
VulkanCapsuleRenderDelegate::update(const uint32_t frameIndex)
{
    this->updateUniforms(frameIndex);
}

std::shared_ptr<Geometry>
VulkanCapsuleRenderDelegate::getGeometry() const
{
    return m_geometry;
}
}