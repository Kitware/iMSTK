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

#include "imstkChartVisualModel.h"
#include "imstkRenderDelegateObjectFactory.h"
#include "imstkVTKRenderDelegate.h"

#include <unordered_map>

using namespace imstk;

namespace imstk
{
class AbstractDataArray;
} // namespace imstk

class vtkPlot;
class vtkTable;
class vtkChartXY;
class vtkContextActor;
class vtkContextScene;
class vtkContextView;

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

//protected:
//    vtkSmartPointer<vtkTable> getTable();

protected:
    vtkSmartPointer<vtkTable>   m_table = nullptr;
    vtkSmartPointer<vtkChartXY> m_chart = nullptr;

    std::unordered_map<std::shared_ptr<Plot2d>, vtkPlot*> m_plots;

    vtkSmartPointer<vtkContextActor> m_chartActor   = nullptr;
    vtkSmartPointer<vtkContextScene> m_contextScene = nullptr;

    int t = 0;
};