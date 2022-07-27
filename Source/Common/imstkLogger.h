/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#ifdef iMSTK_SYNCHRONOUS_LOGGING
#include "imstkLoggerSynchronous.h"

namespace imstk
{
using Logger = LoggerSynchronous;
} // namespace imstk

#else
#include "imstkLoggerG3.h"

namespace imstk
{
using Logger = LoggerG3;
} // namespace imstk

#endif