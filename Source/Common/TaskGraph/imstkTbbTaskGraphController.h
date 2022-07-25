/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTaskGraphController.h"

namespace imstk
{
///
/// \class TbbTaskGraphController
///
/// \brief This class runs an input TaskGraph in parallel using tbb tasks
///
class TbbTaskGraphController : public TaskGraphController
{
public:
    void execute() override;
};
}; // namespace imstk