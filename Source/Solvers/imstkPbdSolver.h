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

#include "imstkSolverBase.h"
#include "imstkPbdObject.h"
#include "imstkGraph.h"

#include <unordered_set>

namespace imstk
{
class PbdObject;
class PbdCollisionConstraint;
///
/// \class PbdSolver
///
/// \brief Position Based Dynamics solver
///
class PbdSolver : public SolverBase
{
using PBDConstraintVector = std::vector<PbdCollisionConstraint*>;

public:
    ///
    /// \brief Constructors/Destructor
    ///
    PbdSolver() = default;
    virtual ~PbdSolver() override = default;

    PbdSolver(const PbdSolver& other) = delete;
    PbdSolver& operator=(const PbdSolver& other) = delete;

    ///
    /// \brief Set MaxIterations. The maximum number of nonlinear iterations.
    ///
    void setMaxIterations(const size_t newMaxIterations) { this->m_maxIterations = newMaxIterations; }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations.
    ///
    size_t getMaxIterations() const { return this->m_maxIterations; }

    ///
    /// \brief
    ///
    void setPbdObject(const std::shared_ptr<PbdObject>& pbdObj) { m_pbdObject = pbdObj; }

    ///
    /// \brief Solve the non linear system of equations G(x)=0 using Newton's method.
    ///
    void solve() override;

    ///
    /// \brief Add the global collision contraints to this solver
    ///
    void addCollisionConstraints(PBDConstraintVector* constraints) { m_PBDConstraints.insert(constraints); }

    ///
    /// \brief Solve the global collision contraints charged to this solver
    ///
    void resolveCollisionConstraints();

private:
    size_t m_maxIterations = 20;                               ///< Maximum number of NL Gauss-Seidel iterations
    std::unordered_set<PBDConstraintVector*> m_PBDConstraints; ///< Collision contraints charged to this solver
    std::shared_ptr<PbdObject> m_pbdObject;
};
} // imstk
