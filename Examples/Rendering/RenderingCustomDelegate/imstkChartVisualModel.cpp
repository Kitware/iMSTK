/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkChartVisualModel.h"
#include "imstkDataArray.h"
#include "imstkLogger.h"

using namespace imstk;

ChartVisualModel::ChartVisualModel()
{
    setDelegateHint("Chart");

    m_viewBounds[0] = 0.0;
    m_viewBounds[1] = 320.0;
    m_viewBounds[2] = 0.0;
    m_viewBounds[3] = 220.0;
}