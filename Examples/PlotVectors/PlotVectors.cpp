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

#include "imstkPlotterUtils.h"

using namespace imstk;

void doVectorPlotting()
{
    Vectord a;
    a.resize(100);
    a.setConstant(1.0001);

    Vectord b;
    b.resize(100);
    b.setConstant(2.0);

    plotterutils::writePlotterVectorMatlab(a, "plotX.m");
    plotterutils::writePlotterVecVsVecMatlab(a, b, "plotXvsY.m");

    plotterutils::writePlotterVectorMatPlotlib(a, "plotX.py");
    plotterutils::writePlotterVecVsVecMatPlotlib(a, b, "plotXvsY.py");

    getchar();
}

int main()
{
    doVectorPlotting();
    return 0;
}
