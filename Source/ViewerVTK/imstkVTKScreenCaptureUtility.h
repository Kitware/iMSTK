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

#include "imstkScreenCaptureUtility.h"

#include <vtkSmartPointer.h>

class vtkPNGWriter;
class vtkRenderWindow;
class vtkWindowToImageFilter;

namespace imstk
{
///
/// \class VTKScreenCaptureUtility
///
/// \brief Utility class to manage screen capture through VTK
///
class VTKScreenCaptureUtility : public ScreenCaptureUtility
{
public:
    using ScreenCaptureUtility::saveScreenShot;

    VTKScreenCaptureUtility(vtkRenderWindow* const rw, const std::string prefix = "Screenshot-");
    ~VTKScreenCaptureUtility() override = default;

    ///
    /// \brief Saves the screenshot as a png file
    ///
    virtual std::string saveScreenShot(const std::string& captureName) override;

protected:
    vtkSmartPointer<vtkWindowToImageFilter> m_windowToImageFilter;
    vtkSmartPointer<vtkPNGWriter> m_pngWriter; //> using vtk's png writer to save the screenshots
    vtkRenderWindow* m_renderWindow;           //> render window whose screen shot will be taken
};
}