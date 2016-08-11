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
    m_translationOffset = m_collidingGeometry->getPosition();
    m_rotationOffset = m_collidingGeometry->getOrientation();
}

void
VirtualCouplingPBDObject::updateFromDevice()
{
    Vec3d p;
    Quatd r;	
	Eigen::Matrix4d t;	
	
    if (!this->computeTrackingData(p, r))
    {
        LOG(WARNING) << "VirtualCouplingPBDObject::updateFromDevice warning: could not update tracking info.";
        return;
    }

    // Update colliding geometry
  m_visualGeometry->setPosition(p);
  m_visualGeometry->setOrientation(r);

   computeTransform(p, r, t);

   transform = t;
  
   Vec4d vertexPos;
   vertexPos.w() = 1;
   Vec3d vertexPos0;
   auto collidingMesh = std::dynamic_pointer_cast<imstk::Mesh>(m_collidingGeometry);
	for (int i = 0; i < collidingMesh->getNumVertices(); ++i)
   {
	   vertexPos0 = collidingMesh->getVertexPosition(i);
	   vertexPos.x() = vertexPos0.x();
	   vertexPos.y() = vertexPos0.y();
	   vertexPos.z() = vertexPos0.z();
	   vertexPos.applyOnTheLeft(transform);
	   vertexPos0.x() = vertexPos.x();
	   vertexPos0.y() = vertexPos.y();
	   vertexPos0.z() = vertexPos.z();	  
	   collidingMesh->setVerticePosition(i, vertexPos0);
   }    
}

void 
VirtualCouplingPBDObject::computeTransform(Vec3d& pos, Quatd& quat, Eigen::Matrix4d& t){
	auto scaling = m_collidingGeometry->getScaling();
	auto angleAxis = Rotd(quat);

	t.setIdentity();
	t(0, 0) = scaling;
	t(1, 1) = scaling;
	t(2, 2) = scaling;
	
	Eigen::Affine3d rot =
		Eigen::Affine3d(Eigen::AngleAxisd(angleAxis.angle() * 180 / M_PI, 
		Eigen::Vector3d(angleAxis.axis()[0],angleAxis.axis()[1],angleAxis.axis()[2])));
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
VirtualCouplingPBDObject::resetCollidingGeo(){

	auto collidingMesh = std::dynamic_pointer_cast<imstk::Mesh>(m_collidingGeometry);
	collidingMesh->setVerticesPositions(collidingMesh->getInitialVerticesPositions());

}

} // imstk
