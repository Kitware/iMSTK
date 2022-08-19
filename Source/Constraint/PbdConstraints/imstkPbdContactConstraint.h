/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdContactConstraint
///
/// \brief A constraint on a rigid body that defines rotationl correction through
/// dx applied at a local position r on the body.
///
/// It is primarily useful for rigid vs rigid and rigid vs deformable collision
/// but can be used for other purposes such as joints.
///
class PbdContactConstraint : public PbdConstraint
{
protected:
    PbdContactConstraint(const int numParticles) :
        PbdConstraint(numParticles), m_r(numParticles), m_weights(numParticles)
    {
    }

public:
    ~PbdContactConstraint() override = default;

    ///
    /// \brief Update positions by projecting constraints.
    ///
    void projectConstraint(PbdState& bodies,
                           const double dt, const SolverType& type) override;

    virtual Vec3d computeRelativeVelocity(PbdState& bodies) { return Vec3d::Zero(); }

    ///
    /// \brief Solve the velocities given to the constraint
    ///
    void correctVelocity(PbdState& bodies, const double dt) override;

    ///
    /// \brief Returns the velocity at the given point on body
    /// Either body in collision could be rigid body
    ///
    Vec3d getVelocityOnRigidBody(PbdState& bodies, const int bodyId, const Vec3d& pt)
    {
        const PbdParticleId pid     = { bodyId, 0 };
        const Vec3d&        bodyPos = bodies.getPosition(pid);
        const Vec3d         r       = pt - bodyPos;

        const Vec3d& v = bodies.getVelocity(pid);
        const Vec3d& w = bodies.getAngularVelocity(pid);
        return v + w.cross(r);
    }

    ///
    /// \brief Get torque magnitude after solve
    ///
    double getTorque(const double dt, const int i)
    {
        const Vec3d force = getForce(dt) * m_dcdx[i];
        return force.cross(m_r[i]).norm();
    }

protected:
    std::vector<Vec3d>  m_r;
    std::vector<double> m_weights;
};

///
/// \class PbdTriangleToBodyConstraint
///
/// \brief Resolves a point on body to a triangle with linear and angular movement
///
class PbdTriangleToBodyConstraint : public PbdContactConstraint
{
public:
    PbdTriangleToBodyConstraint() : PbdContactConstraint(4) { }

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
        const double compliance = 0.0);

    bool computeInterpolantsAndContact(const PbdState& bodies,
                                       std::vector<double>& weights, Vec3d& contactNormal, double& depth) const;

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

    Vec3d computeRelativeVelocity(PbdState& bodies) override;
};

///
/// \class PbdPointToBodyConstraint
///
/// \brief Resolves a point on body to a vertex with linear and angular movement
///
class PbdVertexToBodyConstraint : public PbdContactConstraint
{
public:
    PbdVertexToBodyConstraint() : PbdContactConstraint(2) { }

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
        const double         compliance = 0.0);

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

    Vec3d computeRelativeVelocity(PbdState& bodies) override;
};

///
/// \class PbdEdgeToBodyConstraint
///
/// \brief Resolves a point on body to an edge with linear and angular movement
///
class PbdEdgeToBodyConstraint : public PbdContactConstraint
{
public:
    PbdEdgeToBodyConstraint() : PbdContactConstraint(3) { }

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
        const double compliance = 0.0);

    bool computeInterpolantsAndContact(const PbdState& bodies,
                                       std::vector<double>& weights, Vec3d& contactNormal, double& depth) const;

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

    Vec3d computeRelativeVelocity(PbdState& bodies) override;
};

///
/// \class PbdBodyToBodyDistanceConstraint
///
/// \brief Constrain two locally defined points on each body by a given distance
///
class PbdBodyToBodyDistanceConstraint : public PbdContactConstraint
{
public:
    PbdBodyToBodyDistanceConstraint() : PbdContactConstraint(2) { }

    ///
    /// \brief Initialize the constraint
    /// ptOnBody is global position
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d          ptOnBody0,
        const PbdParticleId& bodyId1,
        const Vec3d          ptOnBody1,
        const double         restLength,
        const double         compliance)
    {
        m_particles[0] = bodyId0;
        // Compute local position on body in rest pose
        m_r[0]      = ptOnBody0 - state.getPosition(bodyId0);
        m_rest_r[0] = state.getOrientation(bodyId0).inverse()._transformVector(m_r[0]);

        m_particles[1] = bodyId1;
        // Compute local position on body
        m_r[1]      = ptOnBody1 - state.getPosition(bodyId1);
        m_rest_r[1] = state.getOrientation(bodyId1).inverse()._transformVector(m_r[1]);

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
    std::array<Vec3d, 2> m_rest_r = { Vec3d::Zero(), Vec3d::Zero() }; // In rest pose
    double m_restLength = 0.0;
};
} // namespace imstk