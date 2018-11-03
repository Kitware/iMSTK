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
