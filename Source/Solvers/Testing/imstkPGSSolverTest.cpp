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

#include "imstkProjectedGaussSeidelSolver.h"
#include "imstkTypes.h"

#include <gtest/gtest.h>

using namespace imstk;

class imstkPGSSolverTest : public ::testing::Test
{
protected:
    ProjectedGaussSeidelSolver<double> m_solver;
};

///
/// \brief TODO
///
TEST_F(imstkPGSSolverTest, Solve5x5)
{
    // Testing Ax=b
    Eigen::MatrixXd Ad(5, 5);
    Ad <<
        1.0, 0.999861, 0.997739, 0.971125, 0.984529,
        0.999861, 1.0, 0.996607, 0.967639, 0.981667,
        0.997739, 0.996607, 1.0, 0.984906, 0.994004,
        0.971125, 0.967639, 0.984906, 1.0, 0.997076,
        0.984529, 0.981667, 0.994004, 0.997076, 1.0;
    Eigen::SparseMatrix<double> A = Ad.sparseView();

    Eigen::VectorXd b(5);
    b(0) = 369.425;
    b(1) = 370.798;
    b(2) = 382.972;
    b(3) = 404.772;
    b(4) = 393.974;
    // Not testing projection here (cu clamps the solution)
    Eigen::MatrixXd cu(5, 2);
    for (int i = 0; i < 5; i++)
    {
        cu(i, 0) = IMSTK_DOUBLE_MIN;
        cu(i, 1) = IMSTK_DOUBLE_MAX;
    }

    m_solver.setA(&A);
    m_solver.setMaxIterations(1000);
    m_solver.setRelaxation(0.05);
    m_solver.setEpsilon(1.0e-8);

    Eigen::VectorXd x = m_solver.solve(b, cu);

    // Check that Ax now equals b
    // Test this way in case multiple solutions exist, here we are only
    // testing that a solution was found
    Eigen::VectorXd bPrime = A.toDense() * x;
    //std::cout << "x: " << x << std::endl << std::endl;
    //std::cout << "Energy " << m_solver.getEnergy() << std::endl;

    //std::cout << "actual b: " << b << std::endl << std::endl;
    //std::cout << "computed b: " << bPrime << std::endl << std::endl;
    for (int i = 0; i < b.size(); i++)
    {
        EXPECT_NEAR(bPrime(i), b(i), 10.0);
    }
}
