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

#pragma once

#include "imstkSphere.h"

#include "imstkVulkanRenderDelegate.h"

#include "vtkSphereSource.h"
#include "vtkPointData.h"
#include "vtkTriangleFilter.h"

namespace imstk
{
class VulkanSphereRenderDelegate : public VulkanRenderDelegate
{
public:

    ///
    /// \brief Default destructor
    ///
    ~VulkanSphereRenderDelegate() = default;

    ///
    /// \brief Default constructor
    ///
    VulkanSphereRenderDelegate(std::shared_ptr<VisualModel> visualModel,
                               SceneObject::Type type,
                               VulkanMemoryManager& memoryManager);

    ///
    /// \brief Update render geometry
    ///
    void update(const uint32_t frameIndex) override;

protected:
    std::vector<std::array<uint32_t, 3>> m_sphereTriangles;
    std::vector<VulkanBasicVertex> m_sphereVertices;
};
}

