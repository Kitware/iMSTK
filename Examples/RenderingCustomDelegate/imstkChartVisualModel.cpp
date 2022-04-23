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