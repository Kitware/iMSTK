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

#include "imstkConjugateGradient.h"

namespace imstk
{
ConjugateGradient::ConjugateGradient()
{
    m_cgSolver.setMaxIterations(m_maxIterations);
    m_cgSolver.setTolerance(m_tolerance);
}

ConjugateGradient::ConjugateGradient(const SparseMatrixd& A, const Vectord& rhs) : ConjugateGradient()
{
    this->setSystem(std::make_shared<LinearSystem<SparseMatrixd>>(A, rhs));
}

void
ConjugateGradient::applyLinearProjectionFilter(Vectord& x, const std::vector<LinearProjectionConstraint>& linProj, const bool setVal)
{
    Vec3d p;
    for (auto &localProjector : linProj)
    {
        const auto threeI = 3 * localProjector.getNodeId();
        p = Vec3d(x(threeI), x(threeI + 1), x(threeI + 2));

        if (!setVal)
        {
            p = localProjector.getProjector()*p;
        }
        else
        {
            p = (Mat3d::Identity() - localProjector.getProjector())*localProjector.getValue();
        }

        x(threeI) = p.x();
        x(threeI + 1) = p.y();
        x(threeI + 2) = p.z();
    }
}

void
ConjugateGradient::solve(Vectord& x)
{
    if(!m_linearSystem)
    {
        LOG(WARNING) << "ConjugateGradient::solve: Linear system is not supplied for CG solver!";
        return;
    }

    if (m_FixedLinearProjConstraints->size() == 0)
    {
        x = m_cgSolver.solve(m_linearSystem->getRHSVector());
    }
    else
    {
        this->modifiedCGSolve(x);
    }
}

void
ConjugateGradient::modifiedCGSolve(Vectord& x)
{
    const auto &b = m_linearSystem->getRHSVector();
    const auto &A = m_linearSystem->getMatrix();

    // Set the initial guess to zero
    x.setZero();
    applyLinearProjectionFilter(x, *m_DynamicLinearProjConstraints, true);
    applyLinearProjectionFilter(x, *m_FixedLinearProjConstraints, true);

    auto res = b;
    applyLinearProjectionFilter(res, *m_DynamicLinearProjConstraints, false);
    applyLinearProjectionFilter(res, *m_FixedLinearProjConstraints, false);
    auto c = res;
    auto delta = res.dot(res);
    auto deltaPrev = delta;
    const auto eps = m_tolerance*m_tolerance*delta;
    double alpha = 0.0;
    double dotval;
    auto q = Vectord(b.size()).setZero();
    size_t iterNum = 0;

    while (delta > eps)
    {
        q = A * c;
        applyLinearProjectionFilter(q, *m_DynamicLinearProjConstraints, false);
        applyLinearProjectionFilter(q, *m_FixedLinearProjConstraints, false);
        dotval = c.dot(q);
        if (dotval != 0.0)
        {
            alpha = delta / dotval;
        }
        else
        {
            LOG(WARNING) << "ConjugateGradient::modifiedCGSolve: deniminator zero. Terminating MCG iteation!";
            return;
        }
        x += alpha * c;
        res -= alpha * q;
        deltaPrev = delta;
        delta = res.dot(res);
        c *= delta / deltaPrev;
        c += res;
        applyLinearProjectionFilter(c, *m_DynamicLinearProjConstraints, false);
        applyLinearProjectionFilter(c, *m_FixedLinearProjConstraints, false);

        if (++iterNum >= m_maxIterations)
        {
            LOG(WARNING) << "ConjugateGradient::modifiedCGSolve - The solver did not converge after max. iterations";
            break;
        }
    }
}

double
ConjugateGradient::getResidual(const Vectord& )
{
    return m_cgSolver.error();
}

void
ConjugateGradient::setTolerance(const double epsilon)
{
    IterativeLinearSolver::setTolerance(epsilon);
    m_cgSolver.setTolerance(epsilon);
}

void
ConjugateGradient::setMaxNumIterations(const size_t maxIter)
{
    IterativeLinearSolver::setMaxNumIterations(maxIter);
    m_cgSolver.setMaxIterations(maxIter);
}

void
ConjugateGradient::setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
    m_cgSolver.compute(m_linearSystem->getMatrix());
}

void
ConjugateGradient::print() const
{
    IterativeLinearSolver::print();

    LOG(INFO) << "Solver: Conjugate gradient";
    LOG(INFO) << "Tolerance: " << m_tolerance;
    LOG(INFO) << "max. iterations: " << m_maxIterations;
}

void
ConjugateGradient::solve(Vectord& x, const double tolerance)
{
    this->setTolerance(tolerance);
    this->solve(x);
}
} // imstk