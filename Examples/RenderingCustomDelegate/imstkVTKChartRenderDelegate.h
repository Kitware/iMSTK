/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkChartVisualModel.h"
#include "imstkVTKRenderDelegate.h"

#include <unordered_map>

class vtkChartXY;
class vtkContextActor;
class vtkContextScene;
class vtkPlot;
class vtkTable;

namespace imstk
{
class AbstractDataArray;
} // namespace imstk

using namespace imstk;

///
/// \class VTKChartRenderDelegate
///
/// \brief Render delegate to render graphs
///
class VTKChartRenderDelegate : public VTKRenderDelegate
{
public:
    VTKChartRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKChartRenderDelegate() override = default;

    ///
    /// \brief Update render delegate source based on the internal data
    ///
    void processEvents() override;

    void updateRenderProperties() override { }

protected:
    vtkSmartPointer<vtkTable>   m_table;
    vtkSmartPointer<vtkChartXY> m_chart;

    std::unordered_map<std::shared_ptr<Plot2d>, vtkPlot*> m_plots;

    vtkSmartPointer<vtkContextActor> m_chartActor;
    vtkSmartPointer<vtkContextScene> m_contextScene;
};