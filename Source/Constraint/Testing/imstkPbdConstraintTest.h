/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc.

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

#include "imstkPbdConstraint.h"

#include <gtest/gtest.h>

///
/// \class PbdConstraintTest
///
/// \brief Test that involves a pbd constraint
///
class PbdConstraintTest : public testing::Test
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
        m_velocities.resize(numParticles);
        m_velocities.fill(imstk::Vec3d::Zero());
        m_invMasses.resize(numParticles);
        m_invMasses.fill(0.0);
    }

    ///
    /// \brief solve the m_constraint
    /// \param dt/timestep is negligible when xPBD not used
    /// \param PBD or xPBD solve type
    ///
    void solve(const double dt, imstk::PbdConstraint::SolverType solverType)
    {
        // Warning: shared_ptr to raw pointer, deleter removed
        m_body->vertices   = std::shared_ptr<imstk::VecDataArray<double, 3>>(&m_vertices, [](auto*) {});
        m_body->velocities = std::shared_ptr<imstk::VecDataArray<double, 3>>(&m_velocities, [](auto*) {});
        m_body->invMasses  = std::shared_ptr<imstk::DataArray<double>>(&m_invMasses, [](auto*) {});
        m_state.m_bodies.push_back(m_body);

        ASSERT_TRUE(m_constraint != nullptr) << "No constraint provided to PbdConstraintTest";
        m_constraint->projectConstraint(m_state, dt, solverType);
    }

public:
    imstk::PbdConstraint* m_constraint     = nullptr;
    std::shared_ptr<imstk::PbdBody> m_body = nullptr;

    imstk::VecDataArray<double, 3> m_vertices;
    imstk::VecDataArray<double, 3> m_velocities;
    imstk::DataArray<double>       m_invMasses;

    imstk::PbdState m_state;
};