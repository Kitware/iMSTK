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

namespace imstk
{
template<typename Scalar>
class ProjectedGaussSeidelSolver
{
public:
    // Sets the vector to be used for the solve
    //void setGuess(Matrix<Scalar, -1, 1>& g) { x = g; }
    void setA(Eigen::SparseMatrix<Scalar>* A) { this->m_A = A; }
    void setMaxIterations(const unsigned int maxIterations) { this->m_maxIterations = maxIterations; }
    void setRelaxation(const Scalar relaxation) { this->m_relaxation = relaxation; }

    Eigen::Matrix<Scalar, -1, 1>& solve(const Eigen::Matrix<Scalar, -1, 1>& b, const Eigen::Matrix<Scalar, -1, 2>& cu)
    {
        const Eigen::SparseMatrix<Scalar>& A = *m_A;

        // Allocate new results
        m_x = Eigen::Matrix<Scalar, -1, 1>(b.rows());
        m_x.setZero();

        // Naive implementation of PGS
        // Consider graph coloring and TBB parallizing
        Eigen::Matrix<Scalar, -1, 1> xOld;
        for (unsigned int i = 0; i < m_maxIterations; i++)
        {
            xOld = m_x;
            for (Eigen::Index r = 0; r < A.rows(); r++)
            {
                Scalar delta = 0.0;

                // Sum up rows (skip r)
                for (Eigen::Index c = 0; c < r; c++)
                {
                    delta += A.coeff(r, c) * m_x[c];
                }
                for (Eigen::Index c = r + 1; c < A.cols(); c++)
                {
                    delta += A.coeff(r, c) * m_x[c];
                }

                delta = (b[r] - delta) / A.coeff(r, r);
                // Apply relaxation factor
                m_x(r) += m_relaxation * (delta - m_x(r));
                // Do projection *every iteration*
                m_x(r) = std::min(cu(r, 1), std::max(cu(r, 0), m_x(r)));
            }

            // Check convergence
            if ((m_x - xOld).norm() < 1.0e-4)
            {
                return m_x;
            }
        }

        return m_x;
    }

private:
    unsigned int m_maxIterations = 3;
    Scalar       m_relaxation    = static_cast<Scalar>(0.1);
    Eigen::Matrix<Scalar, -1, 1> m_x; ///> Results
    Eigen::SparseMatrix<Scalar>* m_A = nullptr;
};
}