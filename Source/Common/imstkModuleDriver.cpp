/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkModuleDriver.h"
#include "imstkModule.h"

namespace imstk
{
void
ModuleDriver::waitForInit()
{
    bool oneModuleIsNotInitYet = true;
    while (oneModuleIsNotInitYet)
    {
        oneModuleIsNotInitYet = false;
        for (auto module : m_modules)
        {
            if (!module->getInit())
            {
                oneModuleIsNotInitYet = true;
            }
        }
    }
}
} // namespace imstk