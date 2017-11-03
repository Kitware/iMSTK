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

#ifndef imstkVulkanRenderDelegate_h
#define imstkVulkanRenderDelegate_h

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <array>

#include "imstkGeometry.h"
#include "imstkVulkanVertexBuffer.h"
#include "imstkVulkanUniformBuffer.h"
#include "imstkVulkanMaterialDelegate.h"

#include "vtkPolyDataMapper.h"

namespace imstk
{
class VulkanRenderDelegate
{
public:

    ///
    /// \brief Default destructor
    ///
    ~VulkanRenderDelegate() = default;

    ///
    /// \brief Creates a render delegate from geometry
    ///
    static std::shared_ptr<VulkanRenderDelegate> make_delegate(
        std::shared_ptr<Geometry> geom,
        VulkanMemoryManager& details);

    ///
    /// \brief Get source geometry
    ///
    virtual std::shared_ptr<Geometry> getGeometry() const = 0;

    ///
    /// \brief Update render geometry. This is implemented a little differently
    ///        in that memory is directly mapped from the Geometry to the
    ///        RenderDelegate.
    ///
    virtual void update(){};

    ///
    /// \brief Initialize memory backing
    ///
    void initialize(VkDevice &device, uint32_t memoryIndex, std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Get vertex buffer
    ///
    std::shared_ptr<VulkanVertexBuffer> getBuffer();

    ///
    /// \brief Get vertex buffer
    ///
    vtkSmartPointer<vtkPolyDataMapper> setUpMapper(vtkAlgorithmOutput * source);

    ///
    /// \brief Initialize data
    ///
    void initializeData(VulkanMemoryManager& memoryManager,
                        std::shared_ptr<RenderMaterial> material);

    ///
    /// \brief Initialize data
    ///
    void updateTransform();

    void updateUniforms();

protected:
    friend class VulkanVertexBuffer;
    friend class VulkanRenderer;
    friend class VulkanUniformBuffer;

    unsigned int m_numTriangles;
    unsigned int m_numVertices;
    unsigned int m_vertexSize;
    double m_loadFactor = 1.0;

    ///
    /// \brief Default constructor (protected)
    ///
    VulkanRenderDelegate(){};

    std::shared_ptr<VulkanVertexBuffer> m_vertexBuffer;

    std::shared_ptr<VulkanUniformBuffer> m_vertexUniformBuffer;
    std::shared_ptr<VulkanUniformBuffer> m_fragmentUniformBuffer;

    std::shared_ptr<VulkanMaterialDelegate> m_material;

    VulkanLocalVertexUniforms m_localVertexUniforms;
    VulkanLocalFragmentUniforms m_localFragmentUniforms;
};
}

#endif