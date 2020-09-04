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

#include "imstkDirectLinearSolver.h"
#include "imstkNew.h"
#include "imstkNewtonSolver.h"

#include <ios>
#include <iostream>
#include <iomanip>

using namespace imstk;

///
/// \brief This example is for demonstration of how to use a NewtonSolver and its verification
///
int
main(int argc, char** argv)
{
    const int N  = 2;
    auto      x  = Vectord(N);
    auto      xe = Vectord(N);
    auto      y  = Vectord(N);
    auto      A  = Matrixd(N, N);

    x[0]  = 100.0;
    x[1]  = 100.0;
    xe[0] = 1.0;
    xe[1] = 10.0;

    auto func = [&y](const Vectord& x, const bool isSemiImplicit) -> const Vectord& {
                    // auto y = Vectord(x.size());
                    y[0] = x[0] * x[0] - 1.0;
                    y[1] = x[1] * x[1] - 100.0;

                    return y;
                };

    auto jac = [&A](const Vectord& x) -> const Matrixd& {
                   // auto A = Matrixd(x.size(), x.size());
                   A(0, 0) = 2 * x[0];
                   A(0, 1) = 0.0;
                   A(1, 0) = 0.0;
                   A(1, 1) = 2 * x[1];

                   return A;
               };

    auto updateX = [&x](const Vectord& du, const bool isSemiImplicit)
                   {
                       x -= du;
                       return;
                   };

    auto updateXold = [](void) {};

    imstkNew<NonLinearSystem<Matrixd>> nlSystem(func, jac);
    nlSystem->setUnknownVector(x);
    nlSystem->setUpdateFunction(updateX);
    nlSystem->setUpdatePreviousStatesFunction(updateXold);

    imstkNew<DirectLinearSolver<Matrixd>> linSolver;
    imstkNew<NewtonSolver<Matrixd>> nlSolver;
    nlSolver->setMaxIterations(100);
    nlSolver->setRelativeTolerance(1e-8);
    nlSolver->setAbsoluteTolerance(1e-10);
    nlSolver->setSystem(nlSystem);
    nlSolver->setLinearSolver(linSolver);

    std::cout << "init_error = " << std::setprecision(12) << std::scientific << (x - xe).norm() << std::endl;
    nlSolver->solve();

    std::cout << "final_error = " << std::setprecision(12) << std::scientific << (x - xe).norm() << std::endl;

    x[0] = 100.0;
    x[1] = 100.0;
    nlSolver->solveGivenState(x);
    std::cout << "final_error = " << std::setprecision(12) << std::scientific << (x - xe).norm() << std::endl;
}
