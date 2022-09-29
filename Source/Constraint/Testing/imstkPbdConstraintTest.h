/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstraint.h"
#include "imstkPbdBody.h"

#include <gtest/gtest.h>

class MultiBodyPbdConstraintTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_state = imstk::PbdState();
    }

    //
    /// \brief Allocate default values for all properties of the PbdBody
    ///
    void setNumParticles(imstk::PbdBody& body, const int numParticles, const bool isRigid)
    {
        body.bodyType = isRigid ? imstk::PbdBody::Type::RIGID : imstk::PbdBody::Type::DEFORMABLE;
        body.vertices = std::make_shared<imstk::VecDataArray<double, 3>>(numParticles);
        body.vertices->fill(imstk::Vec3d::Zero());
        body.orientations = std::make_shared<imstk::StdVectorOfQuatd>(numParticles);
        std::fill_n(body.orientations->begin(), numParticles, imstk::Quatd::Identity());
        body.velocities = std::make_shared<imstk::VecDataArray<double, 3>>(numParticles);
        body.velocities->fill(imstk::Vec3d::Zero());
        body.angularVelocities = std::make_shared<imstk::VecDataArray<double, 3>>(numParticles);
        body.angularVelocities->fill(imstk::Vec3d::Zero());
        body.invMasses = std::make_shared<imstk::DataArray<double>>(numParticles);
        body.invMasses->fill(0.0);
        body.invInertias = std::make_shared<imstk::StdVectorOfMat3d>(numParticles);
        std::fill_n(body.invInertias->begin(), numParticles, imstk::Mat3d::Identity());
    }

    void setNumBodies(const int numBodies)
    {
        m_state.m_bodies.resize(numBodies);
        for (int i = 0; i < numBodies; i++)
        {
            m_state.m_bodies[i] = std::make_shared<imstk::PbdBody>();
        }
    }

    void solve(const double dt, imstk::PbdConstraint::SolverType solverType)
    {
        ASSERT_TRUE(m_constraint != nullptr) << "No constraint provided to PbdConstraintTest";
        m_constraint->projectConstraint(m_state, dt, solverType);
    }

    imstk::PbdConstraint* m_constraint = nullptr;

    imstk::PbdState m_state;
};

///
/// \class PbdConstraintTest
///
/// \brief Test that involves a pbd constraint, assumes single body used
/// in constraint
///
class PbdConstraintTest : public MultiBodyPbdConstraintTest
{
public:
    void SetUp() override
    {
        m_body  = std::make_shared<imstk::PbdBody>();
        m_state = imstk::PbdState();
    }

    ///
    /// \brief Allocate vertices, velocities, and masses
    /// Initialize all to 0's
    ///
    void setNumParticles(const int numParticles)
    {
        m_vertices.resize(numParticles);
        m_vertices.fill(imstk::Vec3d::Zero());
        m_orientations.resize(numParticles);
        std::fill_n(m_orientations.begin(), numParticles, imstk::Quatd::Identity());
        m_velocities.resize(numParticles);
        m_velocities.fill(imstk::Vec3d::Zero());
        m_angularVelocities.resize(numParticles);
        m_angularVelocities.fill(imstk::Vec3d::Zero());
        m_invMasses.resize(numParticles);
        m_invMasses.fill(0.0);
        m_invInterias.resize(numParticles);
        std::fill_n(m_invInterias.begin(), numParticles, imstk::Mat3d::Identity());
    }

    ///
    /// \brief solve the m_constraint
    /// \param dt/timestep is negligible when xPBD not used
    /// \param PBD or xPBD solve type
    ///
    void solve(const double dt, imstk::PbdConstraint::SolverType solverType)
    {
        // Warning: shared_ptr to raw pointer, deleter removed
        m_body->vertices          = std::shared_ptr<imstk::VecDataArray<double, 3>>(&m_vertices, [](auto*) {});
        m_body->orientations      = std::shared_ptr<imstk::StdVectorOfQuatd>(&m_orientations, [](auto*) {});
        m_body->velocities        = std::shared_ptr<imstk::VecDataArray<double, 3>>(&m_velocities, [](auto*) {});
        m_body->angularVelocities = std::shared_ptr<imstk::VecDataArray<double, 3>>(&m_angularVelocities, [](auto*) {});
        m_body->invMasses         = std::shared_ptr<imstk::DataArray<double>>(&m_invMasses, [](auto*) {});
        m_body->invInertias       = std::shared_ptr<imstk::StdVectorOfMat3d>(&m_invInterias, [](auto*) {});
        m_state.m_bodies.push_back(m_body);

        ASSERT_TRUE(m_constraint != nullptr) << "No constraint provided to PbdConstraintTest";
        m_constraint->projectConstraint(m_state, dt, solverType);
    }

    std::shared_ptr<imstk::PbdBody> m_body = nullptr;

    imstk::VecDataArray<double, 3> m_vertices;
    imstk::StdVectorOfQuatd m_orientations;
    imstk::VecDataArray<double, 3> m_velocities;
    imstk::VecDataArray<double, 3> m_angularVelocities;
    imstk::DataArray<double>       m_invMasses;
    imstk::StdVectorOfMat3d m_invInterias;
};