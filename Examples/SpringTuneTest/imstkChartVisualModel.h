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

#include "imstkVisualModel.h"
#include "imstkColor.h"

namespace imstk
{
class AbstractDataArray;
}

using namespace imstk;

struct Plot2d
{
    public:
        std::shared_ptr<AbstractDataArray> m_xVals;
        std::shared_ptr<AbstractDataArray> m_yVals;

        Color m_lineColor  = Color::Red;
        double m_lineWidth = 1.0;
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
    virtual ~ChartVisualModel() override = default;

    void addPlot(Plot2d plot) { m_plots.push_back(std::make_shared<Plot2d>(plot)); }

    const std::vector<std::shared_ptr<Plot2d>>& getPlots() const { return m_plots; }

    void setViewBounds(const Vec4d& bounds) { m_viewBounds = bounds; }
    Vec4d getViewBounds() const { return m_viewBounds; }

protected:
    std::vector<std::shared_ptr<Plot2d>> m_plots;
    Vec4d m_viewBounds;
};