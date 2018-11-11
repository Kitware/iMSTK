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

#include "imstkDebugGeometry.h"

void
imstk::DebugRenderGeometry::setRenderMaterial(std::shared_ptr<RenderMaterial> renderMat)
{
    m_renderMaterial = renderMat;
}

std::shared_ptr<imstk::RenderMaterial>
imstk::DebugRenderGeometry::getRenderMaterial() const
{
    return m_renderMaterial;
}

void
imstk::DebugRenderGeometry::clear()
{
    std::fill(m_vertices.begin(), m_vertices.end(), Vec3d(0., 0., 0.));
    m_dataSize = 0;
}

void
imstk::DebugRenderGeometry::setVertexData(const StdVectorOfVec3d& verts)
{
    if (verts.size() > m_vertices.size())
    {
        LOG(WARNING) << "The size of the vertices supplied is larger than the base size";
    }

    for (unsigned int i = 0; i < verts.size(); ++i)
    {
        m_vertices[i] = verts[i];
    }
    m_dataSize = verts.size();

    //std::fill(m_vertices.begin() + verts.size(), m_vertices.end(), Vec3d(0., 0., 0.));

    m_isModified = true;
}
