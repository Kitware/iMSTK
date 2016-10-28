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

#ifndef imstkPbdSolver_h
#define imstkPbdSolver_h

#include "imstkSolverBase.h"
#include "imstkPbdObject.h"

namespace imstk
{

///
/// \class PbdSolver
///
/// \brief Pbd solver
///
class PbdSolver : public SolverBase
{
public:
    ///
    /// \brief Constructors/Destructor
    ///
    PbdSolver() = default;
    ~PbdSolver() = default;

    PbdSolver(const PbdSolver &other) = delete;
    PbdSolver &operator=(const PbdSolver &other) = delete;

    ///
    /// \brief Set MaxIterations. The maximum number of nonlinear iterations.
    ///
    void setMaxIterations(const size_t newMaxIterations)
    {
        this->m_maxIterations = newMaxIterations;
    }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations.
    ///
    size_t getMaxIterations() const
    {
        return this->m_maxIterations;
    }

    ///
    /// \brief
    ///
    void setPbdObject(const std::shared_ptr<PbdObject>& pbdObj)
    {
        m_pbdObject = pbdObj;
    }

    ///
    /// \brief Solve the non linear system of equations G(x)=0 using Newton's method.
    ///
    void solve()
    {
        SolverBase::solve();

        m_pbdObject->integratePosition();
        m_pbdObject->constraintProjection();
    }

private:
    size_t m_maxIterations = 20;    ///> Maximum number of NL Gauss-Seidel iterations

    std::shared_ptr<PbdObject> m_pbdObject;
};
} // imstk

#endif // imstkPbdSolver_h