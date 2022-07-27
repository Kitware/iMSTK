/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVisualModel.h"
#include "imstkColor.h"

namespace imstk
{
class AbstractDataArray;
} // namespace imstk

using namespace imstk;

struct Plot2d
{
    public:
        std::shared_ptr<AbstractDataArray> xVals;
        std::shared_ptr<AbstractDataArray> yVals;

        Color lineColor  = Color::Red;
        double lineWidth = 1.0;
};

///
/// \class ChartVisualModel
///
/// \brief Class for graphing 2d charts, only supports 2d data
///
class ChartVisualModel : public VisualModel
{
public:
    ChartVisualModel();
    ~ChartVisualModel() override = default;

    void addPlot(Plot2d plot) { m_plots.push_back(std::make_shared<Plot2d>(plot)); }

    const std::vector<std::shared_ptr<Plot2d>>& getPlots() const { return m_plots; }

    void setViewBounds(const Vec4d& bounds) { m_viewBounds = bounds; }
    Vec4d getViewBounds() const { return m_viewBounds; }

protected:
    std::vector<std::shared_ptr<Plot2d>> m_plots;
    Vec4d m_viewBounds;
};