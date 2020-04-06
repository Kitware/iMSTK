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

#include "imstkDebugRenderGeometry.h"

namespace imstk
{
const Vec3d&
DebugRenderGeometry::getVertex(const size_t idx) const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (idx >= getNumVertices())) << "Invalid index";
#endif
    return m_VertexBuffer[idx];
}

void
DebugRenderGeometry::setVertex(const size_t idx, const Vec3d& vert)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (idx >= getNumVertices())) << "Invalid index";
#endif
    m_VertexBuffer[idx] = vert;
}

void
DebugRenderGeometry::setRenderMaterial(const std::shared_ptr<RenderMaterial>& renderMat)
{
    m_renderMaterial = renderMat;
}

const std::shared_ptr<imstk::RenderMaterial>&
DebugRenderGeometry::getRenderMaterial() const
{
    return m_renderMaterial;
}
} // end namespace imstk
