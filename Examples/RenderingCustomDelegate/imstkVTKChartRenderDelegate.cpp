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

#include "imstkVTKChartRenderDelegate.h"
#include "imstkGeometryUtilities.h"

#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkContextActor.h>
#include <vtkContextScene.h>
#include <vtkDataArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkPlot.h>
#include <vtkTable.h>

using namespace imstk;

VTKChartRenderDelegate::VTKChartRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    VTKRenderDelegate(visualModel),
    m_table(vtkSmartPointer<vtkTable>::New()),
    m_chart(vtkSmartPointer<vtkChartXY>::New()),
    m_chartActor(vtkSmartPointer<vtkContextActor>::New()),
    m_contextScene(vtkSmartPointer<vtkContextScene>::New())
{
    auto chartVisualModel = std::dynamic_pointer_cast<ChartVisualModel>(visualModel);

    m_chart->SetAutoSize(false);
    const Vec4d& bounds = chartVisualModel->getViewBounds();
    m_chart->SetSize(vtkRectf(bounds[0], bounds[2], bounds[1], bounds[3]));

    m_contextScene->AddItem(m_chart);
    m_chartActor->SetScene(m_contextScene);
    m_actor = m_chartActor;

    processEvents();
}

void
VTKChartRenderDelegate::processEvents()
{
    auto                                        chartVisualModel = std::dynamic_pointer_cast<ChartVisualModel>(m_visualModel);
    const std::vector<std::shared_ptr<Plot2d>>& plotsImstk       = chartVisualModel->getPlots();

    // If we need to add/remove plots it's best to just to clear all the plots, re-add them
    // So find the diffs

    // Search imstk plots for any plots this doesn't have yet (added)
    std::vector<std::shared_ptr<Plot2d>> plotsToAdd;
    std::vector<std::shared_ptr<Plot2d>> plotsToRemove;
    for (auto plot : plotsImstk)
    {
        // If imstk has the plot but vtk does not
        if (m_plots.count(plot) == 0)
        {
            // Add a new one mapped
            plotsToAdd.push_back(plot);
        }
    }
    // Search this plots for plots imstk doesn't have (removed)
    for (auto pair : m_plots)
    {
        // If not found
        if (std::find(plotsImstk.begin(), plotsImstk.end(), pair.first) == plotsImstk.end())
        {
            // Remove
            plotsToRemove.push_back(pair.first);
        }
    }

    // If there are any diffs (plots added or removed)
    if (plotsToAdd.size() > 0 || plotsToRemove.size() > 0)
    {
        m_chart->ClearPlots();

        for (auto plot : plotsToRemove)
        {
            m_plots.erase(plot);
        }
        for (auto plot : plotsToAdd)
        {
            m_plots[plot] = m_chart->AddPlot(vtkChart::LINE);
        }
    }
    if (m_plots.size() == 0)
    {
        m_chart->ClearPlots();
        return;
    }

    m_chart->SetAutoSize(false);
    const Vec4d& bounds = chartVisualModel->getViewBounds();
    m_chart->SetSize(vtkRectf(bounds[0], bounds[2], bounds[1], bounds[3]));

    // Gather all the arrays and place them in a vtkTable
    std::unordered_map<std::shared_ptr<AbstractDataArray>, size_t> m_arrayLocations;
    for (auto pair : m_plots)
    {
        std::shared_ptr<Plot2d> plotImstk = pair.first;
        if (m_arrayLocations.count(plotImstk->xVals) == 0)
        {
            const size_t i1 = m_arrayLocations.size();
            m_arrayLocations[plotImstk->xVals] = i1;
        }
        if (m_arrayLocations.count(plotImstk->yVals) == 0)
        {
            const size_t i2 = m_arrayLocations.size();
            m_arrayLocations[plotImstk->yVals] = i2;
        }
    }

    // Build the table with no particular order to the columns
    m_table = vtkSmartPointer<vtkTable>::New();
    for (auto arrayKeyValPair : m_arrayLocations)
    {
        vtkSmartPointer<vtkDataArray> arrVtk  = GeometryUtils::copyToVtkDataArray(arrayKeyValPair.first);
        const std::string             arrName = "data" + std::to_string(arrayKeyValPair.second);
        arrVtk->SetName(arrName.c_str());
        m_table->AddColumn(arrVtk);
    }

    Vec2d min = Vec2d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
    Vec2d max = Vec2d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);
    for (auto pair : m_plots)
    {
        vtkPlot*                plotVtk   = pair.second;
        std::shared_ptr<Plot2d> plotImstk = pair.first;

        const size_t xLocation = m_arrayLocations[plotImstk->xVals];
        const size_t yLocation = m_arrayLocations[plotImstk->yVals];

        plotVtk->SetInputData(m_table, xLocation, yLocation);
        const Color& color = plotImstk->lineColor;
        plotVtk->SetColor(color.r * 255.0, color.g * 255.0, color.b * 255.0, color.a * 255.0);
        plotVtk->SetWidth(plotImstk->lineWidth);

        double* rangeX = m_table->GetRowData()->GetArray(static_cast<int>(xLocation))->GetRange();
        double* rangeY = m_table->GetRowData()->GetArray(static_cast<int>(yLocation))->GetRange();

        min[0] = std::min(min[0], rangeX[0]);
        min[1] = std::min(min[1], rangeY[0]);

        max[0] = std::max(max[0], rangeX[1]);
        max[1] = std::max(max[1], rangeY[1]);
    }

    m_chart->GetAxis(1)->SetMinimum(min[0]);
    m_chart->GetAxis(1)->SetMaximum(max[0]);

    m_chart->GetAxis(0)->SetMinimum(min[1]);
    m_chart->GetAxis(0)->SetMaximum(max[1]);
}