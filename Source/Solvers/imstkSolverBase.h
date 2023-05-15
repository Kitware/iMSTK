/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once
#include "imstkDataTracker.h"

namespace imstk
{
///
/// \class SolverBase
///
/// \brief Base class for solvers
///
class SolverBase
{
protected:
    SolverBase() = default;

public:
    virtual ~SolverBase() = default;

    virtual void solve() = 0;

    std::shared_ptr<DataTracker> m_dataTracker;
};
} // namespace imstk