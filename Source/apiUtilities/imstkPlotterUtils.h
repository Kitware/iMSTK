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

#include "imstkMath.h"

#pragma warning( push )
#pragma warning( disable : 4505 ) // Warning C4505 will vanish upon calls to the following functions
namespace imstk
{
namespace plotterutils
{
///
/// \brief Write a Matlab script to plot a given vector
///
void writePlotterVectorMatlab(Vectord& x, const char* fileName);

///
/// \brief Write a Matlab script to plot X vs Y where X, Y are input vectors of same size
///
void writePlotterVecVsVecMatlab(Vectord& x, Vectord& y, const char* fileName);

///
/// \brief Write a MatPlotlib script to plot a given vector
///
void writePlotterVectorMatPlotlib(Vectord& x, const char* fileName);

///
/// \brief Write a MatPlotlib script to plot \p x vs \p y where \p x, \p y are of same
/// size
///
void writePlotterVecVsVecMatPlotlib(Vectord& x, Vectord& y, const char* fileName);
} // plotters
} // imstk

#pragma warning( pop )
