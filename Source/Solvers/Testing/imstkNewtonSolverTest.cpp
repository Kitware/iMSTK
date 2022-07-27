/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkDirectLinearSolver.h"
#include "imstkNew.h"
#include "imstkNewtonSolver.h"

using namespace imstk;

TEST(imstkNewtonSolverTest, Solve)
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

    // Function f(x)=y
    auto func = [&y](const Vectord& x, const bool) -> const Vectord& {
                    // auto y = Vectord(x.size());
                    y[0] = x[0] * x[0] - 1.0;
                    y[1] = x[1] * x[1] - 100.0;

                    return y;
                };

    // Jacobian of function f(x)
    auto funcJacobian = [&A](const Vectord& x) -> const Matrixd& {
                            // auto A = Matrixd(x.size(), x.size());
                            A(0, 0) = 2 * x[0];
                            A(0, 1) = 0.0;
                            A(1, 0) = 0.0;
                            A(1, 1) = 2 * x[1];

                            return A;
                        };

    auto updateX = [&x](const Vectord& du, const bool)
                   {
                       x -= du;
                       return;
                   };

    auto updateXold = [](void) {};

    imstkNew<NonLinearSystem<Matrixd>> nlSystem(func, funcJacobian);
    nlSystem->setUnknownVector(x);
    nlSystem->setUpdateFunction(updateX);
    nlSystem->setUpdatePreviousStatesFunction(updateXold);

    imstkNew<DirectLinearSolver<Matrixd>> linSolver;
    imstkNew<NewtonSolver<Matrixd>>       nlSolver;
    nlSolver->setMaxIterations(100);
    nlSolver->setRelativeTolerance(1e-8);
    nlSolver->setAbsoluteTolerance(1e-10);
    nlSolver->setSystem(nlSystem);
    nlSolver->setLinearSolver(linSolver);

    nlSolver->solve();
    // Should solve to be basically exact for initial x
    EXPECT_NEAR(0.0, (x - xe).norm(), 0.00000000000001);

    // A different x should give near results
    x[0] = 100.0;
    x[1] = 100.0;
    nlSolver->solveGivenState(x);
    EXPECT_NEAR(0.0, (x - xe).norm(), 0.00000001);
}
