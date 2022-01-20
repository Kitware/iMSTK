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

#include "imstkVTKScreenCaptureUtility.h"
#include "imstkLogger.h"

#include <vtkPNGWriter.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>

namespace imstk
{
VTKScreenCaptureUtility::VTKScreenCaptureUtility(vtkRenderWindow* const rw, const std::string prefix /*= "Screenshot-"*/) :
    ScreenCaptureUtility(prefix),
    m_windowToImageFilter(vtkSmartPointer<vtkWindowToImageFilter>::New()),
    m_pngWriter(vtkSmartPointer<vtkPNGWriter>::New())
{
    if (rw != nullptr)
    {
        m_renderWindow = rw;
    }
}

std::string
VTKScreenCaptureUtility::saveScreenShot(const std::string& captureName)
{
    if (m_renderWindow == nullptr)
    {
        LOG(WARNING) << "Render window has not been set yet! ";
        return "";
    }

    if (m_windowToImageFilter->GetInput() == nullptr)
    {
        m_windowToImageFilter->SetInput(m_renderWindow);

        m_windowToImageFilter->SetScale(1);
        m_windowToImageFilter->SetInputBufferTypeToRGB();
        m_windowToImageFilter->ReadFrontBufferOff();
        m_windowToImageFilter->Update();

        m_pngWriter->SetInputConnection(m_windowToImageFilter->GetOutputPort());
    }

    m_windowToImageFilter->Modified();

    std::string filename = captureName + ".png";

    m_pngWriter->SetFileName(filename.data());
    m_pngWriter->Write();

    LOG(INFO) << "Screen shot " << m_screenShotNumber << " saved as " << captureName << "\n";

    return filename;
}
}