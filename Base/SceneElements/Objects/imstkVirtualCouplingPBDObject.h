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

#ifndef imstkVirtualCouplingPBDObject_h
#define imstkVirtualCouplingPBDObject_h

#include "imstkVirtualCouplingObject.h"
#include "imstkPbdRigidObject.h"
namespace imstk
{

class Geometry;
class GeometryMap;

///
/// \class VirtualCouplingPBDObject
///
/// \brief
///
class VirtualCouplingPBDObject : public TrackingController, public PbdRigidObject
{
public:
    ///
    /// \brief Constructor
    ///
	VirtualCouplingPBDObject(std::string name,
		std::shared_ptr<DeviceClient> deviceClient = nullptr,
		double scaling = 1.0) :
		TrackingController(deviceClient, scaling), PbdRigidObject(name)
	{
		m_type = Type::VirtualCoupling;
		m_type = SceneObject::Type::Deformable;		
	}

    ///
    /// \brief Destructor
    ///
    ~VirtualCouplingPBDObject() = default;

	///
	/// \brief Initialize offset based on object geometry
	///
	void initOffsets();

	///
	/// \brief Update geometries transformations
	///
	void updateFromDevice();

	///
	/// \brief Apply forces to the haptic device
	///
	void applyForces();

	///
	/// \brief Get the force to apply to the device
	///
	const Vec3d& getForce() const;

	///
	/// \brief Set the force to apply to the device
	///
	void setForce(Vec3d force);

	///
	/// \brief Reset CollidingGeometry
	///
	void resetCollidingGeo();

	///
	/// \brief compute (offsetted)transform matrix
	///
	void computeTransform(Vec3d& p, Quatd& r, Eigen::Matrix4d& t);

protected:

	bool m_forceModified;
	Vec3d m_force = Vec3d::Zero();
	Eigen::Matrix4d transform = Eigen::Matrix4d();
};


} // imstk

#endif // ifndef imstkVirtualCouplingObject_h
