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

#include <memory>

#include "imstkVirtualCouplingPBDObject.h"
#include "imstkGeometry.h"
#include "imstkGeometryMap.h"

#include <g3log/g3log.hpp>

namespace imstk
{
void
VirtualCouplingPBDObject::initOffsets()
{
    m_translationOffset = m_collidingGeometry->getTranslation();
    m_rotationOffset = m_collidingGeometry->getRotation();
}

void
VirtualCouplingPBDObject::updateFromDevice()
{
    if (!m_trackingDataUptoDate)
    {
        if (!updateTrackingData())
        {
            LOG(WARNING) << "VirtualCouplingPBDObject::updateFromDevice warning: could not update tracking info.";
            return;
        }
    }

    Vec3d p = getPosition();
    Quatd r = Quatd(getRotation());

    // Update colliding geometry
    m_visualGeometry->setTranslation(p);
    m_visualGeometry->setRotation(r);

    computeTransform(p, r, transform);

    auto collidingMesh = std::dynamic_pointer_cast<Mesh>(m_collidingGeometry);

    Vec4d vertexPos4;
    vertexPos4.w() = 1;
    Vec3d vertexPos3;

    for (int i = 0; i < collidingMesh->getNumVertices(); ++i)
    {
        vertexPos3 = collidingMesh->getVertexPosition(i);
        vertexPos4.x() = vertexPos3.x();
        vertexPos4.y() = vertexPos3.y();
        vertexPos4.z() = vertexPos3.z();

        vertexPos4.applyOnTheLeft(transform);
        vertexPos3.x() = vertexPos4.x();
        vertexPos3.y() = vertexPos4.y();
        vertexPos3.z() = vertexPos4.z();

        collidingMesh->setVertexPosition(i, vertexPos3);
    }
    applyCollidingToPhysics();
    updatePbdStates();
}

void
VirtualCouplingPBDObject::computeTransform(Vec3d& pos, Quatd& quat, Eigen::Matrix4d& t)
{
    auto scaling = m_collidingGeometry->getScaling();
    auto angleAxis = Rotd(quat);

    t.setIdentity();
    t(0, 0) = scaling;
    t(1, 1) = scaling;
    t(2, 2) = scaling;

    Eigen::Affine3d rot =
        Eigen::Affine3d(
                Eigen::AngleAxisd(
                    angleAxis.angle() * 180 / PI,
                    Eigen::Vector3d(angleAxis.axis()[0],angleAxis.axis()[1],angleAxis.axis()[2])
                )
            );
    Eigen::Affine3d trans(Eigen::Translation3d(Eigen::Vector3d(pos[0], pos[1], pos[2])));

    t *= trans.matrix();
    t *= rot.matrix();
}

void
VirtualCouplingPBDObject::applyForces()
{
    m_deviceClient->setForce(m_force);
    this->setForce(Vec3d::Zero());
}

const Vec3d&
VirtualCouplingPBDObject::getForce() const
{
    return m_force;
}

void
VirtualCouplingPBDObject::setForce(Vec3d force)
{
    m_force = force;
}

void
VirtualCouplingPBDObject::resetCollidingGeometry()
{
    if (m_collidingGeometry->isMesh())
    {
        auto collidingMesh = std::dynamic_pointer_cast<Mesh>(m_collidingGeometry);
        collidingMesh->setVertexPositions(collidingMesh->getInitialVertexPositions());
    }
    else
    {
        std::cout << "Not a mesh." << std::endl;
    }
}

std::shared_ptr<GeometryMap> VirtualCouplingPBDObject::getColldingToPhysicsMap() const
{
    return m_collidingToPhysicsGeomMap;
}

void VirtualCouplingPBDObject::setColldingToPhysicsMap(std::shared_ptr<GeometryMap> map)
{
    m_collidingToPhysicsGeomMap = map;
}

void VirtualCouplingPBDObject::applyCollidingToPhysics()
{
    if (m_collidingToPhysicsGeomMap && m_physicsGeometry)
    {
        m_collidingToPhysicsGeomMap->apply();
    }
}
} // imstk
