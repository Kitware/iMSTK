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

// STL includes
#include <iostream>
#include <fstream>

// imstk
#include "imstkMath.h"
#include "g3log/g3log.hpp"

namespace imstk
{
namespace plotterutils
{
///
/// \brief Write a Matlab script to plot a given vector
///
static void
writePlotterVectorMatlab(Vectord& x, const char* fileName)
{
    std::ofstream scriptFile(fileName);

    if (!scriptFile.is_open())
    {
        LOG(WARNING) << "Unable to create or open the specified file for plot script!";
        return;
    }

    // print x
    scriptFile << "X=[\n";
    for (auto i = 0; i < x.size(); ++i)
    {
        scriptFile << x[i] << "\n";
    }
    scriptFile << "];\n";

    // plot x
    scriptFile << "plot(X, 'r-', 'LineWidth', 2, 'MarkerSize', 10);\n";
    scriptFile.close();
}

///
/// \brief Write a Matlab script to plot X vs Y where X, Y are input vectors of same size
///
static void
writePlotterVecVsVecMatlab(Vectord& x, Vectord& y, const char* fileName)
{
    // check if the vectors are of the same size
    if (x.size() != y.size())
    {
        LOG(WARNING) << "The vectors supplied for plotting are not of same size!";
        return;
    }

    std::ofstream scriptFile(fileName);

    if (!scriptFile.is_open())
    {
        LOG(WARNING) << "Unable to create or open the specified file for plot script!";
        return;
    }

    // print x
    scriptFile << "X=[\n";
    for (auto i = 0; i < x.size(); ++i)
    {
        scriptFile << x[i] << "\n";
    }
    scriptFile << "];\n";

    // print y
    scriptFile << "Y=[\n";
    for (auto i = 0; i < y.size(); ++i)
    {
        scriptFile << y[i] << "\n";
    }
    scriptFile << "];\n";

    // plot x vs y
    scriptFile << "plot(X, Y, 'r-', 'LineWidth', 2, 'MarkerSize', 10);\n";
    scriptFile.close();
}

///
/// \brief Write a MatPlotlib script to plot a given vector
///
static void
writePlotterVectorMatPlotlib(Vectord& x, const char* fileName)
{
    std::ofstream scriptFile(fileName);

    if (!scriptFile.is_open())
    {
        LOG(WARNING) << "Unable to create or open the specified file for plot script!";
        return;
    }

    // import
    scriptFile << "import numpy as np\n";
    scriptFile << "import matplotlib.pyplot as plt\n";

    // print x
    scriptFile << "X=[\n";
    for (auto i = 0; i < x.size(); ++i)
    {
        scriptFile << x[i] << ",\n";
    }
    scriptFile << "];\n";

    // plot
    scriptFile << "plt.plot(X, 'ro')\n";

    // show the plot
    scriptFile << "plt.show()\n";
}

///
/// \brief Write a MatPlotlib script to plot X vs Y where X, Y are input vectors of same
/// size
///
static void
writePlotterVecVsVecMatPlotlib(Vectord& x, Vectord& y, const char* fileName)
{
    // check if the vectors are of the same size
    if (x.size() != y.size())
    {
        LOG(WARNING) << "The vectors supplied for plotting are not of same size!";
        return;
    }

    std::ofstream scriptFile(fileName);

    if (!scriptFile.is_open())
    {
        LOG(WARNING) << "Unable to create or open the specified file for plot script!";
        return;
    }

    // import
    scriptFile << "import numpy as np\n";
    scriptFile << "import matplotlib.pyplot as plt\n";

    // print x
    scriptFile << "X=[\n";
    for (auto i = 0; i < x.size(); ++i)
    {
        scriptFile << x[i] << ",\n";
    }
    scriptFile << "];\n";

    // print y
    scriptFile << "Y=[\n";
    for (auto i = 0; i < y.size(); ++i)
    {
        scriptFile << y[i] << ",\n";
    }
    scriptFile << "];\n";

    // plot
    scriptFile << "plt.plot(X, Y, 'ro')\n";

    // show the plot
    scriptFile << "plt.show()\n";
}
} // plotters
} // imstk
