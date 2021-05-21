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

#include "imstkDynamicalModel.h"
#include "imstkRigidBodyState.h"
#include "imstkRigidBodyWorld.h"

namespace imstk
{
enum class RigidBodyType
{
    Static,
    Dynamic,
    Kinematic,
    None
};

struct RigidBodyConfig
{
    RigidBodyType m_rigidBodyType = RigidBodyType::Static;
    double m_staticFriction       = 0.01;
    double m_dynamicFriction      = 0.01;
    double m_restitution = 0.01;
};

using namespace physx;

///
/// \class RigidBodyModel
///
/// \brief Implementation of the rigid body dynamics
/// This class interfaces with PhysX rigid body feature
///
class RigidBodyModel : public DynamicalModel<RigidBodyState>
{
public:
    ///
    /// \brief Constructor
    ///
    RigidBodyModel() : DynamicalModel(DynamicalModelType::RigidBodyDynamics)
    {
        m_validGeometryTypes =
        {
            "Plane",
            "Sphere",
            "OrientedBox",
            "SurfaceMesh"
        };
    };
    virtual ~RigidBodyModel() override = default;

public:
    ///
    /// \brief Initialize the physx dynamic model
    ///
    bool initialize() override;

    ///
    /// \brief Configure the model
    ///
    // \todo: Setting of mass and gravity has to happen somewhere.
    void configure(const std::shared_ptr<RigidBodyConfig> matProperty);

    ///
    /// \brief Update the model geometry from the newest rigid body state
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Add local force at a position relative to object
    ///
    void addForce(const Vec3d& force, const Vec3d& pos, bool wakeup = true);

    ///
    /// \brief Is the current object static ? (cannot move).
    ///
    bool isStatic() { return m_isStatic; }

    ///
    /// \brief Set kinematic target of RigidBody
    ///
    void setKinematicTarget(const PxTransform& destination);

    ///
    /// \brief Get the pointer to \ref RigidBodyWorld
    ///
    RigidBodyWorld* getRigidBodyWorld() { return m_rigidBodyWorld; }

    ///
    /// \brief Set the time step size
    ///
    void setTimeStep(const double timeStep)
    {
        if (m_rigidBodyWorld)
        {
            m_rigidBodyWorld->setTimeStep((float)timeStep);
        }
    }

    ///
    /// \brief Returns the time step size
    ///
    double getTimeStep() const { return 0; }

    ///
    /// \brief Reset to initial state
    ///
    void resetToInitialState() override
    {
        if (m_pxDynamicActor)
        {
            auto        p = m_initialState->getPosition().cast<float>();
            Quatf       q(m_initialState->getRotation().rotation().cast<float>());
            PxTransform pose;
            pose.q = PxQuat(q.x(), q.y(), q.z(), q.w());
            pose.p = PxVec3(p.x(), p.y(), p.z());
            m_pxDynamicActor->setGlobalPose(pose);

            // PhysX rbd doesn't support init velocities yet
            m_pxDynamicActor->setLinearVelocity(PxVec3(0.0, 0.0, 0.0));
            m_pxDynamicActor->setAngularVelocity(PxVec3(0.0, 0.0, 0.0));
        }
    }

protected:
    std::shared_ptr<RigidBodyConfig> m_config;
    PxRigidDynamic* m_pxDynamicActor = NULL;
    PxRigidStatic*  m_pxStaticActor  = NULL;
    bool m_isStatic = true;                     ///> Indicates if the body is static or dynamic
    //RigidBodyType m_type = RigidBodyType::none; ///> Indicates if the body is static, dynamic

    Vec3d m_force = Vec3d(0., 0., 0.);
    Vec3d m_forcePos;

private:
    RigidBodyWorld* m_rigidBodyWorld = NULL;

    ///
    /// \brief TODO
    ///
    PxTriangleMesh* createBV34TriangleMesh(PxU32         numVertices,
                                           const PxVec3* vertices,
                                           PxU32         numTriangles,
                                           const PxU32*  indices,
                                           bool          skipMeshCleanup,
                                           bool          skipEdgeData,
                                           bool          inserted,
                                           const PxU32   numTrisPerLeaf);

    ///
    /// \brief TODO
    ///
    void setupCommonCookingParams(PxCookingParams& params, bool skipMeshCleanup, bool skipEdgeData);

    ///
    /// \brief TODO
    ///
    void createSphere();
    ///
    /// \brief TODO
    ///
    void createPlane();
    ///
    /// \brief TODO
    ///
    void createOrientedBox();
    ///
    /// \brief TODO
    ///
    void createMesh();
};
} // imstk
