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

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdContactConstraint
///
/// \brief A constraint on a rigid body that defines dtheta through dx applied
/// at a local position r on the body.
///
/// It is primarily useful for rigid vs rigid and rigid vs deformable collision
/// but can be used for other purposes such as joints.
///
class PbdContactConstraint : public PbdConstraint
{
public:
    enum class ContactType
    {
        DEFORM,
        RIGID
    };

protected:
    PbdContactConstraint(const int numParticles, std::vector<ContactType> contactTypes) :
        PbdConstraint(numParticles)
    {
        CHECK(numParticles == contactTypes.size());
        m_r.resize(numParticles);
    }

public:
    ~PbdContactConstraint() override = default;

    ///
    /// \brief Update positions by projecting constraints.
    ///
    void projectConstraint(PbdState& bodies,
                           const double dt, const SolverType& type) override;

    ///
    /// \brief Solve the velocities given to the constraint
    ///
<<<<<<< HEAD
    void correctVelocity(PbdState& bodies, const double dt) override;
=======
    void correctVelocity(PbdState& bodies) override;
>>>>>>> cbb7c8da (ENH: Correct friction for pbd rbd)

protected:
    std::vector<Vec3d> m_r;
};

///
/// \class PbdTriangleToBodyConstraint
///
/// \brief Resolves a point on body to a triangle with linear and angular movement
///
class PbdTriangleToBodyConstraint : public PbdContactConstraint
{
public:
    PbdTriangleToBodyConstraint() : PbdContactConstraint(4,
                                                         { ContactType::RIGID, ContactType::DEFORM, ContactType::DEFORM, ContactType::DEFORM })
    {
    }

    ///
    /// \brief Initialize the constraint
    /// \param PbdState of bodies
    /// \param Body particle id
    /// \param Global position on the particle body (local computed from it)
    /// \param Point0 of triangle
    /// \param Point1 of triangle
    /// \param Point2 of triangle
    ///
    void initConstraint(
        const PbdState& state,
        const PbdParticleId& bodyId,
        const Vec3d contactPtOnBody,
        const PbdParticleId& x0, const PbdParticleId& x1, const PbdParticleId& x2,
        const double compliance = 0.0)
    {
        m_particles[0] = bodyId;
        // Compute local position on body
        m_r[0] = contactPtOnBody - state.getPosition(bodyId);
        m_particles[1] = x0;
        m_particles[2] = x1;
        m_particles[3] = x2;

        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;
};

///
/// \class PbdPointToBodyConstraint
///
/// \brief Resolves a point on body to a vertex with linear and angular movement
///
class PbdVertexToBodyConstraint : public PbdContactConstraint
{
public:
    PbdVertexToBodyConstraint() : PbdContactConstraint(2,
                                                       { ContactType::RIGID, ContactType::DEFORM })
    {
    }

    ///
    /// \brief Initialize the constraint
    /// \param PbdState of bodies
    /// \param Body particle id
    /// \param Global position on the particle body (local computed from it)
    /// \param Point0 of vertex
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId,
        const Vec3d          contactPtOnBody,
        const PbdParticleId& x0,
        const double         compliance = 0.0)
    {
        m_particles[0] = bodyId;
        // Compute local position on body
        m_r[0] = contactPtOnBody - state.getPosition(bodyId);
        m_particles[1] = x0;

        // Infinite stiffness/completely rigid
        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;
};

///
/// \class PbdEdgeToBodyConstraint
///
/// \brief Resolves a point on body to an edge with linear and angular movement
///
class PbdEdgeToBodyConstraint : public PbdContactConstraint
{
public:
    PbdEdgeToBodyConstraint() : PbdContactConstraint(3,
                                                     { ContactType::RIGID, ContactType::DEFORM, ContactType::DEFORM })
    {
    }

    ///
    /// \brief Initialize the constraint
    /// \param PbdState of bodies
    /// \param Body particle id
    /// \param Global position on the particle body (local computed from it)
    /// \param Point0 of edge
    /// \param Point1 of edge
    ///
    void initConstraint(
        const PbdState& state,
        const PbdParticleId& bodyId,
        const Vec3d contactPtOnBody,
        const PbdParticleId& x0, const PbdParticleId& x1,
        const double compliance = 0.0)
    {
        m_particles[0] = bodyId;
        // Compute local position on body
        m_r[0] = contactPtOnBody - state.getPosition(bodyId);
        m_particles[1] = x0;
        m_particles[2] = x1;

        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;
};

///
/// \class PbdBodyToBodyConstraint
///
/// \brief Resolves contact with contact plane reprojection between two bodies
/// Resolves distance between two points on two bodies by given direction
///
class PbdBodyToBodyConstraint : public PbdContactConstraint
{
public:
    PbdBodyToBodyConstraint() : PbdContactConstraint(2,
                                                     { ContactType::RIGID, ContactType::RIGID })
    {
    }

    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d&         contactPtOnBody0,
        const Vec3d&         contactNormal,
        const PbdParticleId& bodyId1,
        const Vec3d          contactPtOnBody1,
        const double         compliance = 0.0)
    {
        m_particles[0] = bodyId0;
        // Compute local position on body
        m_r[0] = contactPtOnBody0 - state.getPosition(bodyId0);
        m_particles[1] = bodyId1;
        m_r[1] = contactPtOnBody1 - state.getPosition(bodyId1);

        m_contactNormal = contactNormal.normalized();

        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

protected:
    Vec3d m_contactNormal = Vec3d::Zero();
};

///
/// \class PbdBodyToBodyDistanceConstraint
///
/// \brief Constrain two points locally defined on two bodies to maintain
/// a provided distance from each other
///
class PbdBodyToBodyDistanceConstraint : PbdConstraint
{
public:
    PbdBodyToBodyDistanceConstraint() : PbdConstraint()
    {
    }

    ///
    /// \brief ptOnBody are globally defined
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d          ptOnBody0,
        const PbdParticleId& bodyId1,
        const Vec3d          ptOnBody1,
        const double         restLength,
        const double         compliance = 0.0)
    {
        m_particles[0] = bodyId0;
        // Compute local position on body
        m_r[0] = ptOnBody0 - state.getPosition(bodyId0);

        m_particles[1] = bodyId1;
        // Compute local position on body
        m_r[1] = ptOnBody1 - state.getPosition(bodyId1);

        m_restLength = restLength;

        setCompliance(compliance);
    }

    ///
    /// \brief Initialize the constraint
    /// ptOnBody is global position
    /// restLength is set to the initial distance between these two points
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d          ptOnBody0,
        const PbdParticleId& bodyId1,
        const Vec3d          ptOnBody1,
        const double         compliance)
    {
        initConstraint(state, bodyId0, ptOnBody0, bodyId1, ptOnBody1,
            (ptOnBody1 - ptOnBody0).norm(), compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

protected:
    Vec3d  m_r[2]       = { Vec3d::Zero(), Vec3d::Zero() };
    double m_restLength = 0.0;
};
} // namespace imstk