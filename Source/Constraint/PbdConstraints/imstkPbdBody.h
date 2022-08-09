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

#include "imstkMath.h"
#include "imstkVecDataArray.h"

#include <unordered_map>

namespace
{
///
/// \brief Copies src to dest, allocates dest if it does not exist
///
template<typename T>
void
copyAndAllocate(const std::shared_ptr<T>& src, std::shared_ptr<T>& dest)
{
    if (src != nullptr)
    {
        if (dest == nullptr)
        {
            dest = std::make_shared<T>();
        }
        *dest = *src;
    }
}
} // namespace

namespace imstk
{
///
/// \struct PbdBody
///
/// \brief Represents a pbd body in the model. This is a data only object.
/// It does no function.
/// PbdBody can be of different types. The types effect what properties it
/// has.
///
/// A deformable PbdBody only uses vertices, velocities, & masses.
/// A deformable oriented body uses all properties, adding orientations, angular velocities, and inertias.
/// A rigid uses all properties but contains only one particle to represent the rigid body.
///
/// These may all be used together in constraints allowing things like distance constraints to
/// not only function on an edge in a deformable mesh but also between two rigid bodies or even
/// between two totally separate deformable meshes.
///
struct PbdBody
{
    public:
        enum class Type
        {
            DEFORMABLE,
            DEFORMABLE_ORIENTED,
            RIGID
        };

    public:
        PbdBody() : bodyHandle(-1) { }
        PbdBody(const int handle) : bodyHandle(handle) { }

        ///
        /// \brief Deep copy from src, copying dynamic allocations by value
        ///
        void deepCopy(const PbdBody& src)
        {
            fixedNodeInvMass = src.fixedNodeInvMass;
            bodyHandle       = src.bodyHandle;

            copyAndAllocate(src.prevVertices, prevVertices);
            copyAndAllocate(src.vertices, vertices);
            copyAndAllocate(src.velocities, velocities);
            copyAndAllocate(src.masses, masses);
            copyAndAllocate(src.invMasses, invMasses);

            bodyType = src.bodyType;
            if (getOriented())
            {
                copyAndAllocate(src.prevOrientations, prevOrientations);
                copyAndAllocate(src.orientations, orientations);
                copyAndAllocate(src.angularVelocities, angularVelocities);
                copyAndAllocate(src.inertias, inertias);
                copyAndAllocate(src.invInertias, invInertias);
            }

            fixedNodeIds     = src.fixedNodeIds;
            uniformMassValue = src.uniformMassValue;

            externalForce  = src.externalForce;
            externalTorque = src.externalTorque;
        }

        ///
        /// \brief The body should have orientations if its DEFORMABLE_ORIENTED or RIGID
        ///
        bool getOriented() const
        {
            return (bodyType == Type::DEFORMABLE_ORIENTED || bodyType == Type::RIGID);
        }

        void setRigid(const Vec3d& pos,
                      const double mass = 1.0,
                      const Quatd& orientation = Quatd::Identity(),
                      const Mat3d& inertia     = Mat3d::Identity())
        {
            bodyType = PbdBody::Type::RIGID;

            if (vertices == nullptr)
            {
                vertices = std::make_shared<VecDataArray<double, 3>>();
            }
            * vertices = { pos };

            uniformMassValue = mass;

            if (orientations == nullptr)
            {
                orientations = std::make_shared<StdVectorOfQuatd>();
            }
            * orientations = { orientation };

            if (inertias == nullptr)
            {
                inertias = std::make_shared<StdVectorOfMat3d>();
            }
            * inertias = { inertia };
        }

        void setRigidVelocity(const Vec3d& velocity,
                              const Vec3d& angularVelocity = Vec3d::Zero())
        {
            bodyType = PbdBody::Type::RIGID;

            if (velocities == nullptr)
            {
                velocities = std::make_shared<VecDataArray<double, 3>>();
            }
            * velocities = { velocity };

            if (angularVelocities == nullptr)
            {
                angularVelocities = std::make_shared<VecDataArray<double, 3>>();
            }
            * angularVelocities = { angularVelocity };
        }

    public:
        int bodyHandle; ///< Id in the system
        Type bodyType = Type::DEFORMABLE;

        std::shared_ptr<VecDataArray<double, 3>> prevVertices;
        std::shared_ptr<VecDataArray<double, 3>> vertices;

        std::shared_ptr<VecDataArray<double, 3>> velocities;

        std::shared_ptr<DataArray<double>> masses;
        std::shared_ptr<DataArray<double>> invMasses;

        std::shared_ptr<StdVectorOfQuatd> prevOrientations;
        std::shared_ptr<StdVectorOfQuatd> orientations;

        std::shared_ptr<VecDataArray<double, 3>> angularVelocities;

        std::shared_ptr<StdVectorOfMat3d> inertias;
        std::shared_ptr<StdVectorOfMat3d> invInertias;

        /// Nodal/vertex IDs of the nodes that are fixed
        std::vector<int> fixedNodeIds;
        /// Mass properties, not used if per vertex masses are given in geometry attributes
        double uniformMassValue = 1.0;

        Vec3d externalForce  = Vec3d::Zero();
        Vec3d externalTorque = Vec3d::Zero();

        /// Map for archiving fixed nodes' mass.
        std::unordered_map<int, double> fixedNodeInvMass;
};

///
/// \struct PbdState
///
/// \brief Provides interface for accessing particles from a 2d array of PbdBody,Particles
///
struct PbdState
{
    public:
        PbdState() { }

        void deepCopy(const PbdState& src)
        {
            // Copy by value not reference
            m_bodies.resize(src.m_bodies.size());
            for (size_t i = 0; i < m_bodies.size(); i++)
            {
                if (m_bodies[i] == nullptr)
                {
                    m_bodies[i] = std::make_shared<PbdBody>();
                }
                m_bodies[i]->deepCopy(*src.m_bodies[i]);
            }
        }

        inline Vec3d& getPosition(const std::pair<int, int>& bodyParticleId) const { return (*m_bodies[bodyParticleId.first]->vertices)[bodyParticleId.second]; }
        inline Vec3d& getVelocity(const std::pair<int, int>& bodyParticleId) const { return (*m_bodies[bodyParticleId.first]->velocities)[bodyParticleId.second]; }
        inline Quatd& getOrientation(const std::pair<int, int>& bodyParticleId) const { return (*m_bodies[bodyParticleId.first]->orientations)[bodyParticleId.second]; }
        inline Vec3d& getAngularVelocity(const std::pair<int, int>& bodyParticleId) const { return (*m_bodies[bodyParticleId.first]->angularVelocities)[bodyParticleId.second]; }

        inline double getInvMass(const std::pair<int, int>& bodyParticleId) const { return (*m_bodies[bodyParticleId.first]->invMasses)[bodyParticleId.second]; }
        inline Mat3d& getInvInertia(const std::pair<int, int>& bodyParticleId) const { return (*m_bodies[bodyParticleId.first]->invInertias)[bodyParticleId.second]; }

        inline PbdBody::Type getBodyType(const std::pair<int, int>& bodyParticleId) const { return m_bodies[bodyParticleId.first]->bodyType; }

        std::vector<std::shared_ptr<PbdBody>> m_bodies;
};
} // namespace imstk