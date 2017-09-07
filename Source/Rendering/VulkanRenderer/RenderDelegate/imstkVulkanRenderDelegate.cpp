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

#include "imstkVulkanRenderDelegate.h"

#include "g3log/g3log.hpp"

#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkCapsule.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include "imstkVulkanPlaneRenderDelegate.h"
#include "imstkVulkanSphereRenderDelegate.h"
#include "imstkVulkanCubeRenderDelegate.h"
#include "imstkVulkanCapsuleRenderDelegate.h"
#include "imstkVulkanSurfaceMeshRenderDelegate.h"

namespace imstk
{
std::shared_ptr<VulkanRenderDelegate>
VulkanRenderDelegate::make_delegate(std::shared_ptr<Geometry> geom, VulkanMemoryManager& memoryManager)
{
    switch (geom->getType())
    {
    case Geometry::Type::Plane:
    {
        auto plane = std::dynamic_pointer_cast<Plane>(geom);
        return std::make_shared<VulkanPlaneRenderDelegate>(plane, memoryManager);
    }
    case Geometry::Type::Sphere:
    {
        auto sphere = std::dynamic_pointer_cast<Sphere>(geom);
        return std::make_shared<VulkanSphereRenderDelegate>(sphere, memoryManager);
    }
    case Geometry::Type::Cube:
    {
        auto cube = std::dynamic_pointer_cast<Cube>(geom);
        return std::make_shared<VulkanCubeRenderDelegate>(cube, memoryManager);
    }
    case Geometry::Type::Capsule:
    {
        auto capsule = std::dynamic_pointer_cast<Capsule>(geom);
        return std::make_shared<VulkanCapsuleRenderDelegate>(capsule, memoryManager);
    }
    case Geometry::Type::SurfaceMesh:
    {
        auto surfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(geom);
        return std::make_shared<VulkanSurfaceMeshRenderDelegate>(surfaceMesh, memoryManager);
    }
    /*case Geometry::Type::TetrahedralMesh:
    {
        auto tetrahedralMesh = std::dynamic_pointer_cast<TetrahedralMesh>(geom);
        return std::make_shared<VulkanTetrahedralMeshRenderDelegate>(tetrahedralMesh);
    }
    case Geometry::Type::LineMesh:
    {
        LOG(WARNING) << "RenderDelegate::make_delegate error: LineMeshRenderDelegate not yet implemented";
        return nullptr;
    }
    case Geometry::Type::HexahedralMesh:
    {
        LOG(WARNING) << "RenderDelegate::make_delegate error: HexahedralMeshRenderDelegate not yet implemented";
        return nullptr;
    }*/
    default:
    {
        LOG(WARNING) << "RenderDelegate::make_delegate error: Geometry type incorrect.";
        return nullptr;
    }
    }
}

vtkSmartPointer<vtkPolyDataMapper>
VulkanRenderDelegate::setUpMapper(vtkAlgorithmOutput * source)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(source);
    mapper->Update();
    return mapper;
}

std::shared_ptr<VulkanVertexBuffer>
VulkanRenderDelegate::getBuffer()
{
    return m_vertexBuffer;
}

void
VulkanRenderDelegate::initializeData(VulkanMemoryManager& memoryManager, std::shared_ptr<RenderMaterial> material)
{
    m_vertexUniformBuffer = std::make_shared<VulkanUniformBuffer>(memoryManager, (uint32_t)sizeof(VulkanLocalVertexUniforms));
    m_fragmentUniformBuffer = std::make_shared<VulkanUniformBuffer>(memoryManager, (uint32_t)sizeof(VulkanLocalFragmentUniforms));

    m_material = std::make_shared<VulkanMaterialDelegate>(m_vertexUniformBuffer,
        m_fragmentUniformBuffer,
        material,
        memoryManager);

    m_vertexBuffer = std::make_shared<VulkanVertexBuffer>(memoryManager, m_numVertices, m_vertexSize, m_numTriangles);
}

void
VulkanRenderDelegate::updateTransform(std::shared_ptr<Geometry> geometry)
{
    glm::mat4 transform;

    glm::vec3 scale(geometry->getScaling());
    transform = glm::scale(transform, scale);

    auto rotation = geometry->getRotation();
    glm::mat3 rotationMatrix(rotation(0, 0), rotation(0, 1), rotation(0, 2),
                             rotation(1, 0), rotation(1, 1), rotation(1, 2),
                             rotation(2, 0), rotation(2, 1), rotation(2, 2));

    transform = glm::mat4(rotationMatrix) * transform;

    transform[3][0] = geometry->getTranslation().x();
    transform[3][1] = geometry->getTranslation().y();
    transform[3][2] = geometry->getTranslation().z();

    m_localVertexUniforms.transform = transform;
}
}