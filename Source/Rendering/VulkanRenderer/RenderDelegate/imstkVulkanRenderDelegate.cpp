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
#include "imstkDecalPool.h"

#include "imstkVulkanPlaneRenderDelegate.h"
#include "imstkVulkanSphereRenderDelegate.h"
#include "imstkVulkanCubeRenderDelegate.h"
#include "imstkVulkanCapsuleRenderDelegate.h"
#include "imstkVulkanSurfaceMeshRenderDelegate.h"
#include "imstkVulkanDecalRenderDelegate.h"

namespace imstk
{
std::shared_ptr<VulkanRenderDelegate>
VulkanRenderDelegate::make_delegate(std::shared_ptr<Geometry> geom, VulkanMemoryManager& memoryManager)
{
    geom->m_renderDelegateCreated = true;
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
    case Geometry::Type::DecalPool:
    {
        auto decalPool = std::dynamic_pointer_cast<DecalPool>(geom);
        return std::make_shared<VulkanDecalRenderDelegate>(decalPool, memoryManager);
    }
    default:
    {
        LOG(WARNING) << "RenderDelegate::make_delegate error: Geometry type incorrect.";
        geom->m_renderDelegateCreated = false;
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
VulkanRenderDelegate::initializeData(VulkanMemoryManager& memoryManager,
                                     std::shared_ptr<RenderMaterial> material,
                                     VulkanVertexBufferMode mode)
{
    m_vertexUniformBuffer = std::make_shared<VulkanUniformBuffer>(memoryManager, (uint32_t)sizeof(VulkanLocalVertexUniforms));
    m_fragmentUniformBuffer = std::make_shared<VulkanUniformBuffer>(memoryManager, (uint32_t)sizeof(VulkanLocalFragmentUniforms));

    m_material = std::make_shared<VulkanMaterialDelegate>(m_vertexUniformBuffer,
        m_fragmentUniformBuffer,
        material,
        memoryManager);

    m_shadowMaterial = std::make_shared<VulkanMaterialDelegate>(m_vertexUniformBuffer,
        m_fragmentUniformBuffer,
        material,
        memoryManager,
        true);

    m_depthMaterial = std::make_shared<VulkanMaterialDelegate>(m_vertexUniformBuffer,
        m_fragmentUniformBuffer,
        material,
        memoryManager,
        false,
        true);

    m_vertexBuffer = std::make_shared<VulkanVertexBuffer>(memoryManager,
        m_numVertices,
        m_vertexSize,
        m_numTriangles,
        m_loadFactor,
        mode);
}

void
VulkanRenderDelegate::updateTransform()
{
    auto geometry = this->getGeometry();
    if (!geometry->m_transformModified)
    {
        return;
    }
    AffineTransform3d T(geometry->m_transform.matrix());
    T.scale(geometry->getScaling());
    m_localVertexUniforms.transform = glm::make_mat4(T.data());
    geometry->m_transformModified = false;
}

void
VulkanRenderDelegate::updateUniforms(uint32_t frameIndex)
{
    auto geometry = this->getGeometry();
    this->updateTransform();
    m_vertexUniformBuffer->updateUniforms(sizeof(VulkanLocalVertexUniforms),
        (void *)&m_localVertexUniforms,
        frameIndex);

    auto mat = geometry->getRenderMaterial();

    auto color = mat->getDiffuseColor();
    m_localFragmentUniforms.color = glm::vec4(color.r, color.g, color.b, color.a);
    m_localFragmentUniforms.receivesShadows = mat->getReceivesShadows() ? 1 : 0;
    m_localFragmentUniforms.emissivity = mat->getEmissivity();
    m_localFragmentUniforms.roughness = mat->getRoughness();
    m_localFragmentUniforms.metalness = mat->getMetalness();

    m_fragmentUniformBuffer->updateUniforms(sizeof(VulkanLocalFragmentUniforms),
        (void*)&m_localFragmentUniforms, frameIndex);
}
}