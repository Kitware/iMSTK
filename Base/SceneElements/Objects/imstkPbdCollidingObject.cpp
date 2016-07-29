#include "imstkPbdCollidingObject.h"
#include "imstkMesh.h"

namespace imstk {

void PbdCollidingObject::setCollidingGeometry(std::shared_ptr<Geometry> geometry)
{
    m_collidingGeometry = geometry;
    auto mesh = std::static_pointer_cast<Mesh>(m_collidingGeometry);
    m_invMass.resize(mesh->getNumVertices());
}

Vec3d PbdCollidingObject::getVertexPosition(const unsigned int &idx)
{
    auto mesh = std::static_pointer_cast<Mesh>(m_collidingGeometry);
    return mesh->getVertexPosition(idx);
}

void PbdCollidingObject::setVertexPosition(const unsigned int &idx, const Vec3d &v)
{
    auto mesh = std::static_pointer_cast<Mesh>(m_collidingGeometry);
    mesh->setVerticePosition(idx,v);
}

}
