// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef imstkIterativeLinearSystem_h
#define imstkIterativeLinearSystem_h

// iMSTK includes
#include "imstkLinearSolver.h"

namespace imstk
{

///
/// \brief Base class for iterative linear solvers.
///
class IterativeLinearSolver : public LinearSolver<SparseMatrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    IterativeLinearSolver();
    virtual ~IterativeLinearSolver() {};

    ///
    /// \brief Do one iteration of the method.
    ///
    virtual void iterate(Vectord &x, bool updateResidual = true) = 0;

    ///
    /// \brief set/get the maximum number of iterations for the iterative solver.
    ///
    virtual void setMaxNumIterations(const size_t maxIter);
    virtual size_t getMaxNumIterations() const;

    ///
    /// \brief Return residual vector
    ///
    virtual const Vectord& getResidualVector();
    virtual const Vectord& getResidualVector(const Vectord& x);

    ///
    /// \brief Return residue in 2-norm
    ///
    virtual double getResidual(const Vectord &x);

    ///
    /// \brief Print solver information.
    ///
    void print() const override;

    ///
    /// \brief Solve the linear system using Gauss-Seidel iterations.
    ///
    virtual void solve(Vectord &x) override;

    ///
    /// \brief Returns true if the solver is iterative
    ///
    bool isIterative() const override
    {
        return true;
    };

protected:
    size_t m_maxIterations;   ///> Maximum number of iterations to be performed.
    Vectord m_residual;       ///> Storage for residual vector.
};

} //imstk

#endif // imstkIterativeLinearSystem_h
