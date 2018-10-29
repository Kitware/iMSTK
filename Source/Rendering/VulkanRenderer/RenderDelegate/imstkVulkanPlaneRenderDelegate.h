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

#ifndef imstkVulkanPlaneRenderDelegate_h
#define imstkVulkanPlaneRenderDelegate_h

#include "imstkPlane.h"

#include "imstkVulkanRenderDelegate.h"
#include "vtkSphereSource.h"

#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkTriangleFilter.h"

namespace imstk
{
class VulkanPlaneRenderDelegate : public VulkanRenderDelegate
{
public:

    ///
    /// \brief Default destructor
    ///
    ~VulkanPlaneRenderDelegate() = default;

    ///
    /// \brief Default constructor
    ///
    VulkanPlaneRenderDelegate(std::shared_ptr<Plane> plane,
                              SceneObject::Type type,
                              VulkanMemoryManager& memoryManager);

    ///
    /// \brief Update render geometry
    ///
    void update(const uint32_t frameIndex) override;

    ///
    /// \brief Get source geometry
    ///
    std::shared_ptr<Geometry> getGeometry() const override;

protected:
    std::shared_ptr<Plane> m_geometry;

    std::vector<std::array<uint32_t, 3>> m_planeTriangles;
    std::vector<VulkanBasicVertex> m_planeVertices;
};
}

#endif